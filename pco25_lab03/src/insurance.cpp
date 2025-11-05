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
    // Payer autant de factures que possible et les retirer de la liste.
    // Ordre de verrouillage: bills -> money. Appeler who->pay() hors verrous.
    while (true) {
        Seller* whoToPay = nullptr;
        int billToPay = 0;

        mutexBills.lock();
        for (auto it = unpaidBills.begin(); it != unpaidBills.end(); ++it) {
            mutexMoney.lock();
            if (money >= it->second) {
                money -= it->second;
                whoToPay = it->first;
                billToPay = it->second;
                unpaidBills.erase(it);
                mutexMoney.unlock();
                break;
            }
            mutexMoney.unlock();
        }
        mutexBills.unlock();

        if (!whoToPay) break;
        whoToPay->pay(billToPay);
    }
}
