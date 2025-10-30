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

#include <chrono>

#include "pcothread.h"

using namespace std::chrono_literals;

void PcoThread::usleep(uint64_t useconds)
{
    std::this_thread::sleep_for(1us * useconds);
}

std::thread::id PcoThread::getId()
{
    return m_id;
}

void PcoThread::requestStop()
{
    std::lock_guard guard(*m_requestMutex.get());
    m_stopRequested = true;
}

bool PcoThread::stopRequested()
{
    std::lock_guard guard(*m_requestMutex.get());
    return m_stopRequested;
}

void PcoThread::exitThread()
{
    PcoManager::getInstance()->unregisterThread(PcoThread::thisThread());
#ifdef WIN32
    ExitThread(0);
#else
    pthread_exit(0);
#endif
}

PcoThread* PcoThread::thisThread()
{
    return PcoManager::getInstance()->thisThread();
}
