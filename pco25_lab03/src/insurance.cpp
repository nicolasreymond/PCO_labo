#include "insurance.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>


Insurance::Insurance(int uniqueId, int fund) : Seller(fund, uniqueId) {}

void Insurance::run() {
    logger() << "Insurance " <<  uniqueId << " starting with fund " << money << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        // Réception de la somme des cotisations journalières des assurés
        receiveContributions();

        // Payer les factures
        payBills();

        clock->worker_end_day();
    }

    logger() << "Insurance " <<  uniqueId << " stopping with fund " << money << std::endl;
}

void Insurance::receiveContributions() {
    constexpr int nbContributions = 1; // TODO : Find a way to get the number of contributions
    mutexMoney.lock();
    this->money += nbContributions * INSURANCE_CONTRIBUTION;
    mutexMoney.unlock();

}

void Insurance::invoice(int bill, Seller* who) {
    mutexBills.lock();
    unpaidBills.emplace_back(who, bill);
    mutexBills.unlock();
}

void Insurance::payBills() {
    mutexBills.lock();
    for (const auto& [who, bill] : unpaidBills) {
        mutexMoney.lock();
        if (money >= bill) {
            money -= bill;
            mutexMoney.unlock();
            who->pay(bill);
        } else {
            mutexMoney.unlock();
        }
    }
    mutexBills.unlock();
}
