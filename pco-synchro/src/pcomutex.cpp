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

#include "pcomutex.h"
#include "pcomanager.h"


PcoMutex::PcoMutex(PcoMutex::RecursionMode recursionMode) : m_recursionMode(recursionMode)
{
}

void PcoMutex::lock()
{
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::MutexLock);
    if (m_recursionMode == RecursionMode::Recursive) {
        m_recursiveMutex.lock();
    }
    else {
        m_mutex.lock();
    }
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::MutexLock);
}

void PcoMutex::unlock()
{
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::MutexUnlock);
    if (m_recursionMode == RecursionMode::Recursive) {
        m_recursiveMutex.unlock();
    }
    else {
        m_mutex.unlock();
    }
    PcoManager::getInstance()->randomSleep(PcoManager::EventType::MutexUnlock);
}
