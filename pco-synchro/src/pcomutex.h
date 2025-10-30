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

#ifndef PCOMUTEX_H
#define PCOMUTEX_H

#include <mutex>

///
/// \brief The PcoMutex class
///
/// This class offers a classical Mutex implementation.
/// The mutex can be recursive or not, and the lock() and unlock() methods
/// can add random sleeps before and after the effective lock() and unlock().
///
class PcoMutex
{
public:


    ///
    /// \brief The RecursionMode enum
    ///
    enum RecursionMode { Recursive, NonRecursive };


    ///
    /// \brief PcoMutex constructor
    /// \param recursionMode Indicates if the mutex is recursive or not
    ///
    PcoMutex(RecursionMode recursionMode = RecursionMode::NonRecursive);

    /// No copy
    PcoMutex (const PcoMutex&) = delete;

    /// No copy
    PcoMutex (const PcoMutex&&) = delete;

    /// No copy
    PcoMutex& operator= ( const PcoMutex & ) = delete;

    /// Default destructor
    ~PcoMutex() = default;

    ///
    /// \brief Locks the mutex
    ///
    /// This function tries to get the mutex. If it is already locked by
    /// another thread, then the caller is blocked until the mutex
    /// is unlocked.
    ///
    /// In case the mutex has been initialized as a recursive one,
    /// consecutive calls to lock by the same thread will not block.
    ///
    void lock();

    ///
    /// \brief Unlocks the mutex
    ///
    /// This function unlocks the mutex. If at least one thread is waiting
    /// for the mutex, one of them is awaken and gains the mutex.
    ///
    void unlock();

protected:

    /// A standard mutex, when initialized as a non-recursive mutex
    std::mutex m_mutex;

    /// A recursive mutex, when initialized as a recursive mutex
    std::recursive_mutex m_recursiveMutex;

    /// Indicates if the mutex is recursive or not (not recursive by default)
    const RecursionMode m_recursionMode;
};

#endif // PCOMUTEX_H
