#include "clinic.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>
#include <random>

Clinic::Clinic(int id, int fund, std::vector<ItemType> resourcesNeeded)
: Seller(fund, id), resourcesNeeded(std::move(resourcesNeeded)) {
    for (auto it : this->resourcesNeeded) {
        stocks[it] = 0;
    }

    stocks[ItemType::SickPatient] = 0;
    stocks[ItemType::RehabPatient] = 0;
}

void Clinic::run() {
    // start message removed per request (use stderr debug prints only)

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested())
            break;

        payBills();

        // Essayer de traiter le prochain patient (peut commander des ressources)
        processNextPatient();


        // Transférer les patients déjà traités vers un hôpital pour leur réhabilitation
        sendPatientsToRehab();

        clock->worker_end_day();
    }

}


int Clinic::transfer(ItemType what, int qty) {
    // Les cliniques n'acceptent que les transferts de patients malades
    if (what != ItemType::SickPatient){
        return 0;
    }

    // Si la clinique n'a pas de factures impayées et a des fonds, accepter les patients
    if (getFund() <= 0){
        return 0;
    }

    // Si la clinique n'a pas de factures impayées et a des fonds, accepter les patients
    mutexBill.lock();
    bool hasUnpaid = !unpaidBills.empty();
    mutexBill.unlock();

    if (hasUnpaid){
        return 0;
    }

    // Accepter tous les patients malades demandés (protéger le stock)
    mutexStock.lock();
    stocks[ItemType::SickPatient] += qty;
    mutexStock.unlock();
    return qty;

}

bool Clinic::hasResourcesForTreatment() const {
    for (auto item : resourcesNeeded) {
        if (stocks.at(item) <= 0) {
            return false;
        }
    }
    return true;

}

void Clinic::payBills() {
    // Copy unpaid bills locally to avoid holding `mutex` while calling external
    // Supplier::pay(), which could cause re-entrancy or cross-locking deadlocks.
    mutexBill.lock();
    auto localBills = unpaidBills; // copy
    mutexBill.unlock();

    // For each bill, try to claim funds and pay the supplier without holding
    // the clinic-wide `mutex`. If payment succeeds, remove it from the shared
    // unpaidBills list under lock.
    for (const auto &entry : localBills) {
        Supplier* sup = entry.first;
        int bill = entry.second;

        // Try to atomically reserve the money for this bill
        bool reserved = false;
        mutexMoney.lock();
        if (money >= bill) {
            money -= bill;
            reserved = true;
        }
        mutexMoney.unlock();

        if (!reserved) continue;

        // Pay supplier outside of clinic locks
        sup->pay(bill);

        // Remove one matching bill from unpaidBills
        mutexBill.lock();
        for (auto it = unpaidBills.begin(); it != unpaidBills.end(); ++it) {
            if (it->first == sup && it->second == bill) {
                unpaidBills.erase(it);
                break;
            }
        }
        mutexBill.unlock();
    }
}

void Clinic::processNextPatient() {
    // Si des patients malades attendent, essayez d'en traiter un. Si des ressources
    // manquent, essayez de les commander.
    mutexStock.lock();
    int waiting = stocks[ItemType::SickPatient];
    mutexStock.unlock();

    if (waiting > 0) {
        if (hasResourcesForTreatment()) {
            treatOne();
        } else {
            orderResources();
        }
    }

}

void Clinic::sendPatientsToRehab() {
    mutexStock.lock();
    int toSend = stocks[ItemType::RehabPatient];
    mutexStock.unlock();
    if (toSend <= 0) return;
    if (hospitals.empty() || insurance == nullptr) return;

    // Essayer de transférer tous les patients en réhabilitation vers le premier hôpital
    Seller* hosp = hospitals.front();
    int admitted = hosp->transfer(ItemType::RehabPatient, toSend);
    if (admitted > 0) {
        mutexStock.lock();
        stocks[ItemType::RehabPatient] -= admitted;
        mutexStock.unlock();

        int bill = admitted * getCostPerService(ServiceType::Treatment);
        // Invoicer l'assurance ; l'assurance appellera pay() sur la clinique
        if (insurance) insurance->invoice(bill, this);
    }

}

void Clinic::orderResources() {
    // Si des factures sont en attente, ne pas commander de ressources
    mutexBill.lock();
    bool hasUnpaid = !unpaidBills.empty();
    mutexBill.unlock();
    if (hasUnpaid) return;

    // Pour chaque ressource requise, si la clinique n'en a pas, en commander une unité
    for (auto item : resourcesNeeded) {
        mutexStock.lock();
        int have = stocks[item];
        mutexStock.unlock();

        if (have <= 0) {
            Supplier* sup = chooseRandomSupplier(item);
            if (!sup) continue;
            int bill = sup->buy(item, 1);
            if (bill > 0) {
                // Ajouter la ressource (protéger les stocks)
                mutexStock.lock();
                stocks[item] += 1;
                mutexStock.unlock();

                // Enregistrer la facture impayée
                mutexBill.lock();
                unpaidBills.emplace_back(sup, bill);
                mutexBill.unlock();
            }
        }
    }

}

void Clinic::treatOne() {
    // S'assurer qu'il y a au moins un patient malade et des ressources + fonds
    mutexStock.lock();
    if (stocks[ItemType::SickPatient] <= 0) {
        mutexStock.unlock();
        return;
    }
    int salary = getEmployeeSalary(EmployeeType::TreatmentSpecialist);

    mutexMoney.lock();
    if (money < salary) {
        mutexMoney.unlock();
        mutexStock.unlock();
        return;
    }
    money -= salary;
    mutexMoney.unlock();

    // Consommer les ressources
    for (auto it : resourcesNeeded) {
        stocks[it] -= 1;
    }

    // Mettre à jour les stocks de patients
    stocks[ItemType::SickPatient] -= 1;
    stocks[ItemType::RehabPatient] += 1;
    // Increment the paid-employees counter while still holding the stock mutex
    nbEmployeesPaid++;
    // Per-type counters are not available in this build; omit nbEmployeesPaidByType update.
    mutexStock.unlock();

}

void Clinic::pay(int bill) {

    mutexMoney.lock();
    money += bill;
    mutexMoney.unlock();

}

Supplier *Clinic::chooseRandomSupplier(ItemType item) {
    std::vector<Supplier*> availableSuppliers;

    // Sélectionner les Suppliers qui ont la ressource recherchée
    for (Seller* seller : suppliers) {
        auto* sup = dynamic_cast<Supplier*>(seller);
        if (sup->sellsResource(item)) {
            availableSuppliers.push_back(sup);
        }
    }

    // Choisir aléatoirement un Supplier dans la liste
    assert(availableSuppliers.size());
    std::vector<Supplier*> out;
    std::sample(availableSuppliers.begin(), availableSuppliers.end(), std::back_inserter(out),
            1, std::mt19937{std::random_device{}()});
    return out.front();
}

void Clinic::setHospitalsAndSuppliers(std::vector<Seller*> hospitals, std::vector<Seller*> suppliers) {
    this->hospitals = hospitals;
    this->suppliers = suppliers;
}

void Clinic::setInsurance(Seller* ins) { 
    insurance = ins; 
}


int Clinic::getTreatmentCost() {
    return 0;
}

int Clinic::getWaitingPatients() {
    return stocks[ItemType::SickPatient];
}

int Clinic::getNumberPatients() {
    return stocks[ItemType::SickPatient] + stocks[ItemType::RehabPatient];
}

Pulmonology::Pulmonology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::Pill, ItemType::Thermometer}) {}

Cardiology::Cardiology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::Syringe, ItemType::Stethoscope}) {}

Neurology::Neurology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::Pill, ItemType::Scalpel}) {}
