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
    // If there is no sick patient or no clinic to send the patients to, nothing to do
    if (sickPatients == 0 || clinics.empty()) {
        mutexStock.unlock();
        return;
    }
    mutexClinics.lock();
    // For each clinic, try to transfer as many sick patients as possible
    for (const auto &clinic : clinics) {
        if (sickPatients > 0) {
            const int transferred = clinic->transfer(ItemType::SickPatient, sickPatients);
            insurance->invoice(transferred * getCostPerService(ServiceType::PreTreatmentStay), this);
            stocks[ItemType::SickPatient] -= transferred;
            sickPatients -= transferred;
        } else {
            // No more sick patients to transfer, we can stop
            break;
        }
    }
    mutexClinics.unlock();
    mutexStock.unlock();

}

void Hospital::updateRehab() {

    //For each patient in rehab, we decrease their days left by 1 and count how many have completed rehab
    mutexRehab.lock();
    const int completedPatients = rehabPatientsPerDaysLeft[1];
    rehabPatientsPerDaysLeft[1] = 0;
    for (int daysLeft = 2; daysLeft <=5; ++daysLeft) {
        rehabPatientsPerDaysLeft[daysLeft - 1] = rehabPatientsPerDaysLeft[daysLeft];
    }
    rehabPatientsPerDaysLeft[5] = 0;
    mutexRehab.unlock();

    // We can free the beds and invoice the insurance for completed patients
    mutexFreed.lock();
    nbFreed += completedPatients;
    mutexFreed.unlock();
    insurance->invoice(completedPatients * REHAB_COST, this);

    // Now we remove the completed patients from stock
    mutexStock.lock();
    stocks[ItemType::RehabPatient] -= completedPatients;
    mutexStock.unlock();
}

void Hospital::payNursingStaff() {
    mutexMoney.lock();
    const int totalSalary = nbNursingStaff * getEmployeeSalary(EmployeeType::NursingStaff);
    money -= totalSalary;
    nbEmployeesPaid += nbNursingStaff;
    mutexMoney.unlock();

}

void Hospital::pay(int bill) {
    mutexMoney.lock();
    this->money += bill;
    mutexMoney.unlock();
}

int Hospital::transfer(ItemType what, int qty) {
    // If the hospital cannot pay its nursing staff, it cannot accept new patients
    mutexMoney.lock();
    if (money <= nbNursingStaff * getEmployeeSalary(EmployeeType::NursingStaff)) {
        mutexMoney.unlock();
        return 0;
    }
    mutexMoney.unlock();

    if (what == ItemType::SickPatient) {
        // We can only accept patients if there are available beds
        const int availableBeds = maxBeds - getNumberPatients();
        const int acceptedQty = std::min(qty, availableBeds);
        mutexStock.lock();
        stocks[ItemType::SickPatient] += acceptedQty;
        mutexStock.unlock();
        return acceptedQty;
    }

    if (what == ItemType::RehabPatient) {
        const int availableBeds = maxBeds - getNumberPatients();
        const int acceptedQty = std::min(qty, availableBeds);
        mutexStock.lock();
        stocks[ItemType::RehabPatient] += acceptedQty;
        mutexStock.unlock();

        // We also need to update the rehab patients map to track their rehab days
        mutexRehab.lock();
        rehabPatientsPerDaysLeft[5] += acceptedQty;
        mutexRehab.unlock();

        return acceptedQty;
    }

    // The hospital only accepts SickPatient and RehabPatient transfers, so for other item types we return 0
    return 0;

}

int Hospital::getNumberPatients() {
    mutexStock.lock();
    mutexFreed.lock();
    const int numPatients = stocks[ItemType::SickPatient] + stocks[ItemType::RehabPatient] + nbFreed;
    mutexFreed.unlock();
    mutexStock.unlock();
    return numPatients;
}

void Hospital::setClinics(std::vector<Seller*> c) {
    clinics = std::move(c);
}

void Hospital::setInsurance(Seller* ins) { 
    insurance = ins; 
}
