// ambulance.cpp
#include "ambulance.h"
#include <pcosynchro/pcothread.h>


Ambulance::Ambulance(int id, int fund,
                     std::vector<ItemType> resourcesSupplied,
                     std::map<ItemType,int> initialStocks)
: Seller(fund, id), resourcesSupplied(resourcesSupplied) {
    for (auto it : resourcesSupplied) {
        stocks[it] = initialStocks.count(it) ? initialStocks[it] : 0;
    }
}

void Ambulance::run() {
    logger() << "Ambulance " <<  uniqueId << " starting with fund " << money << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        sendPatients();

        clock->worker_end_day();
    }

    logger() << "Ambulance " <<  uniqueId << " stopping with fund " << money << std::endl;
}

void Ambulance::sendPatients() {
    // Determine the number of patients to send
    const int nbPatientsToTransfer = this->getNumberPatients();
    if (nbPatientsToTransfer <= 0) {
        return; // No patient, thus nothing to do
    }

    // Choose a random hospital
    mutexHospitals.lock();
    auto* hospital = chooseRandomSeller(hospitals);
    mutexHospitals.unlock();

    const int staffSalary = getEmployeeSalary(EmployeeType::EmergencyStaff);

    mutexMoney.lock();
    // Checks if the ambulance can pay the employee for the transfer
    if (staffSalary <= money) {
        // Pay the employee
        money -= staffSalary;
        nbEmployeesPaid += 1;
        mutexMoney.unlock();

        // Transfer the patients
        const int transferredPatients = hospital->transfer(ItemType::SickPatient, nbPatientsToTransfer);
        mutexStock.lock();
        stocks.at(ItemType::SickPatient) -= transferredPatients;
        mutexStock.unlock();

        // Bill the insurance
        const int billAmount = transferredPatients * getCostPerService(ServiceType::Transport);
        insurance->invoice(billAmount, this);
    } else {
        // Not enough money, transfer is cancelled
        mutexMoney.unlock();
    }
}

void Ambulance::pay(int bill) {
    mutexMoney.lock();
    this->money += bill;
    mutexMoney.unlock();
}

void Ambulance::setHospitals(std::vector<Seller*> h) {
    mutexHospitals.lock();
    hospitals = std::move(h);
    mutexHospitals.unlock();
}

void Ambulance::setInsurance(Seller* ins) { 
    insurance = ins; 
}

int Ambulance::getNumberPatients() {
    mutexStock.lock();
    const int nbPatient = stocks[ItemType::SickPatient];
    mutexStock.unlock();
    return nbPatient;
}
