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

#ifndef PCOSEMAPHORE_H
#define PCOSEMAPHORE_H

#include <mutex>
#include <condition_variable>
#include <queue>

class PcoManager;

///
/// \brief The PcoSemaphore class
///
/// This class offers a strong semaphore, with a waiting queue managed in
/// FIFO order. (A weak semaphore does not have a FIFO queue).
///
class PcoSemaphore
{
public:

    ///
    /// \brief PcoSemaphore
    /// \param n The initial value of the semaphore, a positive integer
    /// \param monitor Indicates if the blocked thread list has to be monitored
    ///
    /// The second parameter allows to monitor the status of the waiting list.
    /// If yes, then a monitoring object (interacting with the PcoManager)
    /// is noticed whenever a thread blocks on this semaphore.
    ///
    PcoSemaphore(unsigned int n = 0, bool monitor = true);

    /// No copy
    PcoSemaphore (const PcoSemaphore&) = delete;

    /// No copy
    PcoSemaphore (const PcoSemaphore&&) = delete;

    /// No copy
    PcoSemaphore& operator= ( const PcoSemaphore & ) = delete;

    /// Destructor
    ~PcoSemaphore();

    ///
    /// \brief Acquires the semaphore
    ///
    /// This method decrements the semaphore value. If the new value is less
    /// than 0, the caller is blocked and is put in the waiting queue.
    /// It can continue when a release() allows it to continue.
    ///
    /// The waiting queue is FIFO.
    ///
    void acquire();

    ///
    /// \brief Releases the semaphore
    ///
    /// This method increments the semaphore. If the new value is less or equal
    /// to 0, then the first thread of the waiting queue is released and can
    /// continue.
    ///
    void release();

protected:

    /// A queue of condition variables for the waiting queue
    std::queue<std::condition_variable*> m_waitingCondition;

    /// An internal mutex to protect the semaphore value
    std::mutex m_mutex;

    /// The semaphore value
    int m_value;

    /// Indicates if the semaphore's waiting list is monitored
    bool m_monitor;

    /// PcoManager is a friend, to simplify its development
    friend PcoManager;
};


#endif // PCOSEMAPHORE_H
