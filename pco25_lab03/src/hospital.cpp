// hospital.cpp
#include "hospital.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

Hospital::Hospital(int id, int fund, int maxBeds)
: Seller(fund, id), maxBeds(maxBeds), nbNursingStaff(maxBeds) {
    stocks[ItemType::SickPatient] = 0;
    stocks[ItemType::RehabPatient] = 0;
}

void Hospital::run() {
    logger() << "Hospital " <<  uniqueId << " starting with fund " << money << ", maxBeds " << maxBeds << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        transferSickPatientsToClinic();
        updateRehab();
        payNursingStaff();

        clock->worker_end_day();
    }

    logger() << "Hospital " <<  uniqueId << " stopping with fund " << money << std::endl;
}

void Hospital::transferSickPatientsToClinic() {
    mutexStock.lock();
    int sickPatients = stocks[ItemType::SickPatient];
    if (sickPatients == 0 || clinics.empty()) {
        mutexStock.unlock();
        return;
    }
    mutexClinics.lock();
    for (const auto &clinic : clinics) {
        if (sickPatients > 0) {
            const int transferred = clinic->transfer(ItemType::SickPatient, sickPatients);
            insurance->invoice(transferred * getCostPerService(ServiceType::PreTreatmentStay), this);
            stocks[ItemType::SickPatient] -= transferred;
            sickPatients -= transferred;
        } else {
            break;
        }
    }
    mutexClinics.unlock();
    mutexStock.unlock();

}

void Hospital::updateRehab() {

    // Pour les patients en rééducation, on décale le nombre de patients selon les jours restants
    mutexRehab.lock();
    const int completedPatients = rehabPatientsPerDaysLeft[1];
    rehabPatientsPerDaysLeft[1] = 0;
    for (int daysLeft = 2; daysLeft <=5; ++daysLeft) {
        rehabPatientsPerDaysLeft[daysLeft - 1] = rehabPatientsPerDaysLeft[daysLeft];
    }
    rehabPatientsPerDaysLeft[5] = 0;
    mutexRehab.unlock();

    // On libère les lits des patients ayant terminé leur rééducation
    mutexFreed.lock();
    nbFreed += completedPatients;
    mutexFreed.unlock();
    insurance->invoice(completedPatients * REHAB_COST, this);

    // On met à jour le stock des patients en rééducation
    mutexStock.lock();
    stocks[ItemType::RehabPatient] -= completedPatients;
    mutexStock.unlock();
}

void Hospital::payNursingStaff() {
    // Pas besoin de protéger le nbNursingStaff, parce qu'il n'est jamais modifié
    mutexMoney.lock();
    mutexEmployees.lock();
    const int totalSalary = nbNursingStaff * getEmployeeSalary(EmployeeType::NursingStaff);
    money -= totalSalary;
    nbEmployeesPaid += nbNursingStaff;
    mutexEmployees.unlock();
    mutexMoney.unlock();

}

void Hospital::pay(int bill) {
    mutexMoney.lock();
    this->money += bill;
    mutexMoney.unlock();
}

int Hospital::transfer(ItemType what, int qty) {
    // Si l'hôpital n'a pas assez de fonds pour payer le personnel, il ne prend plus de transfer
    mutexMoney.lock();
    if (money <= nbNursingStaff * getEmployeeSalary(EmployeeType::NursingStaff)) {
        mutexMoney.unlock();
        return 0;
    }
    mutexMoney.unlock();

    if (what == ItemType::SickPatient) {
        const int availableBeds = maxBeds - getNumberPatients();
        const int acceptedQty = std::min(qty, availableBeds);
        mutexStock.lock();
        stocks[ItemType::SickPatient] += acceptedQty;
        mutexStock.unlock();
        return acceptedQty;
    } else if (what == ItemType::RehabPatient) {
        const int availableBeds = maxBeds - getNumberPatients();
        const int acceptedQty = std::min(qty, availableBeds);
        mutexStock.lock();
        stocks[ItemType::RehabPatient] += acceptedQty;
        mutexStock.unlock();

        // On ajoute les patients en rééducation avec 5 jours restants
        mutexRehab.lock();
        rehabPatientsPerDaysLeft[5] += acceptedQty;
        mutexRehab.unlock();

        return acceptedQty;
    } else {
        return 0;
    }

}

int Hospital::getNumberPatients() {
    return stocks[ItemType::SickPatient] + stocks[ItemType::RehabPatient] + nbFreed;
}

void Hospital::setClinics(std::vector<Seller*> c) {
    clinics = std::move(c);
}

void Hospital::setInsurance(Seller* ins) { 
    insurance = ins; 
}
