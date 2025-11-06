// authors: Nicolas Reymond, Nadia Cattin
#include "supplier.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>


Supplier::Supplier(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied)
    : Seller(fund, uniqueId), resourcesSupplied(resourcesSupplied) {
    for (const auto& item : resourcesSupplied) {    
        stocks[item] = 0;
    }
}

void Supplier::run() {
    logger() << "Supplier " <<  uniqueId << " starting with fund " << money << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        attemptToProduceResource();

        clock->worker_end_day();
    }

    logger() << "Supplier " <<  uniqueId << " stopping with fund " << money << std::endl;
}

void Supplier::attemptToProduceResource() {
    const ItemType type = getRandomItemFromStock();
    const int salary = getEmployeeSalary(EmployeeType::Supplier);
    if (money >= salary) {
        stocks[type] += 1;
        money -= salary;
        nbEmployeesPaid += 1;
    }
}

int Supplier::buy(ItemType it, int qty) {
    mutexStock.lock();
    const int availableQty = stocks[it];
    const int qtyToSell = std::min(qty, availableQty);
    stocks[it] -= qtyToSell;
    mutexStock.unlock();
    return qtyToSell * getCostPerUnit(it);
}

void Supplier::pay(int bill) {
    mutexMoney.lock();
    money += bill;
    mutexMoney.unlock();
}

int Supplier::getMaterialCost() {
    int totalCost = 0;
    for (const auto& item : resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

bool Supplier::sellsResource(ItemType item) const {
    return std::find(resourcesSupplied.begin(), resourcesSupplied.end(), item) != resourcesSupplied.end();
}
