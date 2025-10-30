// ambulance.cpp
#include "ambulance.h"
#include "costs.h"
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
    // Déterminer le nombre de patients à envoyer
    const int nbPatientsToTransfer = this->getNumberPatients();
    if (nbPatientsToTransfer <= 0) {
        return; // Aucun patient à transférer, rien à faire
    }

    // Choisir un hôpital au hasard
    auto* hospital = chooseRandomSeller(hospitals);

    const int staffSalary = getEmployeeSalary(EmployeeType::EmergencyStaff);

    mutexMoney.lock();
    mutexEmployees.lock();
    // Vérifier si l'ambulance peut payer un employé pour le transfert
    if (staffSalary <= money) {
        // Payer l'employé
        money -= staffSalary;
        nbEmployeesPaid += 1;
        mutexEmployees.unlock();
        mutexMoney.unlock();

        // Transférer les patients
        const int transferredPatients = hospital->transfer(ItemType::SickPatient, nbPatientsToTransfer);
        mutexStock.lock();
        stocks.at(ItemType::SickPatient) -= transferredPatients;
        mutexStock.unlock();

        // Facturer l'assurance
        const int billAmount = transferredPatients * getCostPerService(ServiceType::Transport);
        insurance->invoice(billAmount, this);
    } else {
        // Ne peut pas payer un employé, transfert annulé
        mutexEmployees.unlock();
        mutexMoney.unlock();
    }
}

void Ambulance::pay(int bill) {
    mutexMoney.lock();
    this->money += bill;
    mutexMoney.unlock();
}

void Ambulance::setHospitals(std::vector<Seller*> h) {
    hospitals = std::move(h);
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
