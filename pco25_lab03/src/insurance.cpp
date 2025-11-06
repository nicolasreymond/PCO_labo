// authors: Nicolas Reymond, Nadia Cattin
#include "insurance.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>


Insurance::Insurance(int uniqueId, int fund) : Seller(fund, uniqueId) {
}

void Insurance::run() {
    logger() << "Insurance " << uniqueId << " starting with fund " << money << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        receiveContributions();

        payBills();

        clock->worker_end_day();
    }

    logger() << "Insurance " << uniqueId << " stopping with fund " << money << std::endl;
}

void Insurance::receiveContributions() {
    this->money += INSURANCE_CONTRIBUTION;
}

void Insurance::invoice(int bill, Seller *who) {
    mutexBills.lock();
    unpaidBills.emplace_back(who, bill);
    mutexBills.unlock();
}

void Insurance::payBills() {
    // The while loop continues until no more bills can be paid, it allows searching for a payable bill while protecting
    // the bills, and pay it without holding the lock on the bills
    while (true) {
        Seller *whoToPay = nullptr;
        int billToPay = 0;

        mutexBills.lock();
        // Find the first bill that can be paid with the available money
        for (auto it = unpaidBills.begin(); it != unpaidBills.end(); ++it) {
            if (money >= it->second) {
                money -= it->second;
                whoToPay = it->first;
                billToPay = it->second;
                unpaidBills.erase(it);
                break;
            }
        }
        mutexBills.unlock();

        // If no bill can be paid, stop searching
        if (!whoToPay) break;
        // Pay the bill
        whoToPay->pay(billToPay);
    }
}
