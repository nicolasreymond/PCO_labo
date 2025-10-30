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

#include "pcoconditionvariable.h"

#include "pcomanager.h"

PcoConditionVariable::PcoConditionVariable(bool monitor) : m_monitor(monitor)
{}

void PcoConditionVariable::wait(PcoMutex *mutex)
{
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionWait);
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_nbWaiting ++;
        // It is very important to keep this unlock within the critical section
        // protected by m_mutex, so the unlock() and the waiting are kind of
        // an atomic operation
        mutex->unlock();

        if (m_monitor) {
            PcoManager::getInstance()->addWaitingThread();
        }

        m_waitingCondition.wait(lk);
    }
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionWait);
    mutex->lock();
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionWait);
}


bool PcoConditionVariable::waitForSeconds(PcoMutex *mutex, int seconds)
{
    bool result = true;
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionWait);
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_nbWaiting ++;
        // It is very important to keep this unlock within the critical section
        // protected by m_mutex, so the unlock() and the waiting are kind of
        // an atomic operation
        mutex->unlock();

        if (m_monitor) {
            PcoManager::getInstance()->addWaitingThread();
        }

        if (m_waitingCondition.wait_for(lk, std::chrono::seconds(seconds)) == std::cv_status::timeout) {
            result = false;
        }
    }
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionWait);
    mutex->lock();
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionWait);
    return result;
}

void PcoConditionVariable::notifyOne()
{
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionNotify);
    m_mutex.lock();
    if (m_nbWaiting > 0) {
        m_nbWaiting --;
        m_waitingCondition.notify_one();

        if (m_monitor) {
            PcoManager::getInstance()->removeWaitingThread();
        }

    }
    m_mutex.unlock();
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionNotify);
}


void PcoConditionVariable::notifyAll()
{
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionNotifyAll);
    m_mutex.lock();
    if (m_nbWaiting > 0) {
        for (int i = 0; i < m_nbWaiting; i++) {

            if (m_monitor) {
                PcoManager::getInstance()->removeWaitingThread();
            }

        }
        m_nbWaiting = 0;
        m_waitingCondition.notify_all();
    }
    m_mutex.unlock();
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::WaitConditionNotifyAll);
}

