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

#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <stdexcept>

#include "pcomanager.h"
#include "pcothread.h"
#include "pcosemaphore.h"

PcoManager *PcoManager::getInstance()
{
    static PcoManager pcoManager;
    return &pcoManager;
}

PcoManager::PcoManager()
{
    m_sleepMutex.lock();
    m_usecondsMap[EventType::Standard] = 0.0;
    m_sleepMutex.unlock();
}

PcoManager::~PcoManager()
{
    m_sleepMutex.lock();
    m_usecondsMap.clear();
    m_sleepMutex.unlock();
    m_mutex.lock();
    m_runningThreads.clear();
    m_mutex.unlock();
}

void PcoManager::setMaxSleepDuration(unsigned int useconds, EventType eventType)
{
    m_sleepMutex.lock();
    m_usecondsMap[eventType] = useconds;
    m_sleepMutex.unlock();
}

void PcoManager::randomSleep(EventType eventType)
{
    unsigned int useconds;
    m_sleepMutex.lock();
    if (m_usecondsMap.count(eventType) > 0) {
        useconds = m_usecondsMap[eventType];
    }
    else {
        useconds = m_usecondsMap[EventType::Standard];
    }
    static std::random_device rd;  //Will be used to obtain a seed for the random number engine
    static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(0, static_cast<int>(useconds));
    auto randomValue = dis(gen);
    std::chrono::microseconds value(randomValue);
    m_sleepMutex.unlock();
    std::this_thread::sleep_for(value);
}


void PcoManager::registerThread(PcoThread *thread)
{
    m_mutex.lock();
    m_runningThreads[thread->getId()] = thread;
    m_mutex.unlock();
}

void PcoManager::unregisterThread(PcoThread *thread)
{
    m_mutex.lock();
    m_runningThreads.erase(thread->getId());
    m_mutex.unlock();
}

void PcoManager::registerSemaphore(PcoSemaphore *semaphore)
{
    m_mutex.lock();
    m_semaphores.push_back(semaphore);
    m_mutex.unlock();
}

void PcoManager::unregisterSemaphore(PcoSemaphore *semaphore)
{
    m_mutex.lock();
    auto it = m_semaphores.begin();
    while (it != m_semaphores.end()) {
        if (*it == semaphore) {
            m_semaphores.erase(it);
            m_mutex.unlock();
            return;
        }
        it++;
    }
    m_mutex.unlock();
}

PcoManager::Mode PcoManager::getMode()
{
    m_mutex.lock();
    auto result = m_mode;
    m_mutex.unlock();
    return result;
}

void PcoManager::setNormalMode()
{
    m_mutex.lock();
    m_mode = Mode::Normal;
    m_mutex.unlock();
}

void PcoManager::setFreeMode()
{
    m_mutex.lock();
    m_mode = Mode::Free;
    m_mutex.unlock();
    for(auto &sem : m_semaphores) {
        while (sem->m_value <= 0) {
            sem->release();
        }
    }
}

PcoThread* PcoManager::thisThread()
{
    m_mutex.lock();
    auto currentId = std::this_thread::get_id();
    try {
        PcoThread *result = m_runningThreads.at(currentId);
        m_mutex.unlock();
        return result;
    }
    catch (std::out_of_range&) {
        m_mutex.unlock();
        return nullptr;
    }
}

#include <iostream>

void PcoManager::addWaitingThread()
{
    m_mutex.lock();
    m_nbBlockedThreads ++;
    int nbBlocked = m_nbBlockedThreads;
    if (m_watchDog != nullptr) {
        m_watchDog->trigger(nbBlocked);
    }
    m_mutex.unlock();
}

void PcoManager::removeWaitingThread()
{
    m_mutex.lock();
    m_nbBlockedThreads --;
    m_mutex.unlock();
}

int PcoManager::nbBlockedThreads()
{
    m_mutex.lock();
    int result = m_nbBlockedThreads;
    m_mutex.unlock();
    return result;
}

void PcoManager::setWatchDog(PcoWatchDog *watchDog)
{
    m_watchDog = watchDog;
}
