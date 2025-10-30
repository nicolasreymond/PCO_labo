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

#ifndef PCOCONDITIONVARIABLE_H
#define PCOCONDITIONVARIABLE_H

#include <mutex>
#include <condition_variable>

#include "pcomutex.h"

///
/// \brief The PcoConditionVariable class
///
/// This class implements a condition variable, to be used to synchronize
/// threads.
///
class PcoConditionVariable
{
public:

    /// \brief PcoConditionVariable
    /// \param monitor Indicates if the blocked thread list has to be monitored
    ///
    /// The only parameter allows to monitor the status of the waiting list.
    /// If yes, then a monitoring object (interacting with the PcoManager)
    /// is noticed whenever a thread blocks on this semaphore.
    PcoConditionVariable(bool monitor = true);

    /// No copy
    PcoConditionVariable (const PcoConditionVariable&) = delete;

    /// No copy
    PcoConditionVariable (const PcoConditionVariable&&) = delete;

    /// No copy
    PcoConditionVariable& operator= ( const PcoConditionVariable & ) = delete;

    ///
    /// \brief Blocks the current thread
    /// \param mutex The mutex to unlock() and to lock() again
    ///
    /// This method blocks the caller. Before blocking, it unlocks the mutex
    /// passed as argument.
    /// When the thread is awaken, it has to compete to reaquire the mutex
    /// before continuing.
    ///
    void wait(PcoMutex *mutex);

    ///
    /// \brief notifies the condition variables.
    ///
    /// This method wakes up a thread blocked on the condition variable if such
    /// a thread exists. If the waiting queue is empty, nothing happens.
    ///
    void notifyOne();

    ///
    /// \brief notifies all threads waiting on the condition variables.
    ///
    /// This method wakes up all thread blocked on the condition variable if
    /// there is at least one thread in the queue. All the thread then have to
    /// compete to gain the mutex.
    ///
    void notifyAll();


    ///
    /// \brief Blocks the current thread for at most a certain duration
    /// \param mutex The mutex to unlock() and to lock() again
    /// \param seconds The maximum number of seconds to wait
    ///
    /// This method blocks the caller. Before blocking, it unlocks the mutex
    /// passed as argument.
    /// When the thread is awaken, it has to compete to reaquire the mutex
    /// before continuing.
    /// The thread waits for at most a certain amount of seconds. If the
    /// thread has not been awakened during that duration, the function
    /// returns
    ///
    bool waitForSeconds(PcoMutex *mutex, int seconds);

protected:

    /// Condition variable used to block the threads
    std::condition_variable m_waitingCondition;

    /// Mutex to protect the internal variable
    std::mutex m_mutex;

    /// Number of waiting threads. Not really necessary, but good for optimization
    int m_nbWaiting{0};

    /// Indicates if the condition variable's waiting list is monitored
    bool m_monitor;

};

#endif // PCOCONDITIONVARIABLE_H
