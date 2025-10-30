/*****************************************************************************
 * Copyright (C) 2020 HEIG-VD                                                *
 *                                                                           *
 * This file is part of PcoSynchro.                                          *
 *                                                                           *
 * PcoSynchro is free software: you can redistribute it and/or modify        *
 * it under the terms of the GNU Lesser General Public License as published  *
 * by the Free Software Foundation, either version 3 of the License, or      *
 * (at your option) any later version.                                       *
 *                                                                           *
 * PcoSynchro is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Lesser General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with PcoSynchro.  If not, see <https://www.gnu.org/licenses/>.      *
 *****************************************************************************/

#include <iostream>

#include "pcosemaphore.h"
#include "pcomanager.h"


PcoSemaphore::PcoSemaphore(unsigned int n, bool monitor) : m_value(static_cast<int>(n)), m_monitor(monitor)
{
    if (m_monitor) {
        PcoManager::getInstance()->registerSemaphore(this);
    }
}

PcoSemaphore::~PcoSemaphore()
{
    if (m_monitor) {
        PcoManager::getInstance()->unregisterSemaphore(this);
    }
    std::unique_lock<std::mutex> acquire(m_mutex);
    if (m_value < 0) {
        std::cout << "A PcoSemaphore should not be deleted if a thread is waiting on it" << std::endl;
    }
    while (!m_waitingCondition.empty()) {
        m_waitingCondition.front()->notify_one();
        m_waitingCondition.pop();
    }
}

void PcoSemaphore::acquire()
{
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::SemaphoreAcquire);
    {
        std::unique_lock<std::mutex> acquire(m_mutex);
        if (m_monitor && PcoManager::getInstance()->getMode() == PcoManager::Mode::Free) {
            return;
        }
        m_value --;
        if (m_value < 0) {
            std::condition_variable *variable = new std::condition_variable();
            m_waitingCondition.emplace(variable);
            if (m_monitor) {
                PcoManager::getInstance()->addWaitingThread();
            }
            variable->wait(acquire);
            delete variable;
        }
    }

    PcoManager::getInstance()->randomSleep(PcoManager::EventType::SemaphoreAcquire);
}

void PcoSemaphore::release()
{
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::SemaphoreRelease);
    {
        std::unique_lock<std::mutex> acquire(m_mutex);
        m_value ++;
        if (m_value <= 0) {
            m_waitingCondition.front()->notify_one();
            m_waitingCondition.pop();
            if (m_monitor) {
                PcoManager::getInstance()->removeWaitingThread();
            }
        }
    }

    PcoManager::getInstance()->randomSleep(PcoManager::EventType::SemaphoreRelease);
}

