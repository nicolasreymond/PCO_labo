// authors: Nicolas Reymond, Nadia Cattin
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
    // Boucle quotidienne: payBills -> processNextPatient -> sendPatientsToRehab -> payBills

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested())
            break;


        // Traiter un patient (peut commander des ressources)
        processNextPatient();


        // Transférer les patients traités en rééducation
        sendPatientsToRehab();

        // Régler les factures créées pendant la journée
        payBills();

        clock->worker_end_day();
    }

}


int Clinic::transfer(ItemType what, int qty) {
    // Accepte uniquement SickPatient, si fonds > 0 et aucune facture impayée
    if (what != ItemType::SickPatient){
        return 0;
    }

    if (getFund() <= 0){
        return 0;
    }

    mutexBill.lock();
    bool hasUnpaid = !unpaidBills.empty();
    mutexBill.unlock();

    if (hasUnpaid){
        return 0;
    }

    // Accepter et mettre à jour le stock
    mutexStock.lock();
    stocks[ItemType::SickPatient] += qty;
    mutexStock.unlock();
    return qty;

}

bool Clinic::hasResourcesForTreatment() const {
    // Toutes les ressources requises doivent être > 0
    for (auto item : resourcesNeeded) {
        if (stocks.at(item) <= 0) {
            return false;
        }
    }
    return true;

}

void Clinic::payBills() {
    // Règle les factures impayées; ordre de verrouillage: bills -> money.
    // Réserver les fonds, retirer la facture, puis payer hors verrous.
    while (true) {
        Supplier* supToPay = nullptr;
        int billToPay = 0;
        bool found = false;

        // Parcourir les factures et vérifier la trésorerie
        mutexBill.lock();
        for (auto it = unpaidBills.begin(); it != unpaidBills.end(); ++it) {
            // Lire/modifier `money` en sécurité
            mutexMoney.lock();
            if (money >= it->second) {
                supToPay = it->first;
                billToPay = it->second;
                // Réserver les fonds et retirer la facture
                money -= billToPay;
                unpaidBills.erase(it);
                mutexMoney.unlock();
                found = true;
                break;
            }
            mutexMoney.unlock();
        }
        mutexBill.unlock();

        if (!found) break; // Rien à payer

        // Payer le fournisseur hors section critique
        supToPay->pay(billToPay);
    }
}

void Clinic::processNextPatient() {
    // Si des patients attendent: commander si nécessaire, puis traiter
    mutexStock.lock();
    int waiting = stocks[ItemType::SickPatient];
    mutexStock.unlock();

    if (waiting > 0) {
        if (!hasResourcesForTreatment()) {
            // Commander les ressources manquantes
            orderResources();
        }
        // Traiter si prêt
        if (hasResourcesForTreatment()) {
            treatOne();
        }
    }

}

void Clinic::sendPatientsToRehab() {
    // Transférer tous les patients de rééducation et facturer l'assurance
    mutexStock.lock();
    int toSend = stocks[ItemType::RehabPatient];
    mutexStock.unlock();
    if (toSend <= 0) return;
    if (hospitals.empty() || insurance == nullptr) return;

    // Vers le premier hôpital
    Seller* hosp = hospitals.front();
    int admitted = hosp->transfer(ItemType::RehabPatient, toSend);
    if (admitted > 0) {
        mutexStock.lock();
        stocks[ItemType::RehabPatient] -= admitted;
        mutexStock.unlock();

        int bill = admitted * getCostPerService(ServiceType::Treatment);
        // Invoicer l'assurance (qui paiera la clinique)
        if (insurance) insurance->invoice(bill, this);
    }

}

void Clinic::orderResources() {
    // Ne pas commander si des factures sont en attente
    mutexBill.lock();
    bool hasUnpaid = !unpaidBills.empty();
    mutexBill.unlock();
    if (hasUnpaid) return;

    // Pour chaque ressource manquante: commander 1 unité et enregistrer la facture
    for (auto item : resourcesNeeded) {
        mutexStock.lock();
        int have = stocks[item];
        mutexStock.unlock();

        if (have <= 0) {
            Supplier* sup = chooseRandomSupplier(item);
            if (!sup) continue;
            int bill = sup->buy(item, 1);
            if (bill > 0) {
                // Ajouter la ressource
                mutexStock.lock();
                stocks[item] += 1;
                mutexStock.unlock();

                // Enregistrer la facture pour payBills()
                mutexBill.lock();
                unpaidBills.emplace_back(sup, bill);
                mutexBill.unlock();
            }
        }
    }

}

void Clinic::treatOne() {
    // Traiter un patient si ressources et salaire disponibles
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
    // Incrémenter le compteur d'employés payés
    nbEmployeesPaid++;
    mutexStock.unlock();

}

void Clinic::pay(int bill) {
    // Créditer la clinique (thread-safe)
    mutexMoney.lock();
    money += bill;
    mutexMoney.unlock();

}

Supplier *Clinic::chooseRandomSupplier(ItemType item) {
    std::vector<Supplier*> availableSuppliers;

    // Filtrer les suppliers qui vendent l'item
    for (Seller* seller : suppliers) {
        auto* sup = dynamic_cast<Supplier*>(seller);
        if (sup->sellsResource(item)) {
            availableSuppliers.push_back(sup);
        }
    }

    // Choisir aléatoirement un Supplier (au moins 1 attendu)
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
