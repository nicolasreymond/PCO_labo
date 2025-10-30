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

#ifndef PCOTHREAD_H
#define PCOTHREAD_H

#include <thread>
#include <memory>
#include <functional>

#include "pcomanager.h"

//template <class T>
//std::decay_t<T> decay_copy(T&& v) { return std::forward<T>(v); }

// The following lines check if we really use at least C++17.
// We need that for std::invoke()
#ifndef __has_include
#error "You need C++17 to compile pcosynchro"
#endif


///
/// \brief The PcoThread class
///
/// This class is meant to be used to start a new thread, and to join it.
///
class PcoThread
{
public:

    ///
    /// \brief The thread constructor (and starter).
    /// \param fn The function to be run by the new thread
    /// \param args The arguments to be sent to the function
    /// The constructor creates and starts the thread automatically.
    /// A random sleep can be added within the new thread, thanks to the
    /// PcoManager.
    ///
    /// This constructor is a template, allowing to pass any type of function
    /// and any type of arguments to this function.
    /// It corresponds to the same constructor as the one of std::thread :
    /// http://www.cplusplus.com/reference/thread/thread/
    ///
    /// The usage of std::invoke allows to pass a member function and an
    /// object to apply the member function, as well as standard functions
    /// and arguments. It requires c++17 to compile.
    ///
    /// From http://www.cplusplus.com/reference/thread/thread/ :
    /// The arguments to the thread function are moved or copied by value.
    /// If a reference argument needs to be passed to the thread function,
    /// it has to be wrapped (e.g., with std::ref or std::cref).
    ///
    template <class Fn, class... Args>
    explicit PcoThread (Fn&& fn, Args&&... args)
    {
        m_requestMutex = std::make_unique<std::mutex>();
        m_thread = std::make_unique<std::thread>([=](){
            m_id = std::this_thread::get_id();
            PcoManager::getInstance()->registerThread(this);
            PcoManager::getInstance()->randomSleep(PcoManager::EventType::ThreadCreation);
            //std::invoke(decay_copy(std::forward<Fn>(fn)),
            //            decay_copy(std::forward<Args>(args))...);
            std::invoke(fn,args...);
            PcoManager::getInstance()->unregisterThread(this);
        });
    }

    /// No copy
    PcoThread (const PcoThread&) = delete;

    /// No copy
    PcoThread (PcoThread&&) = delete;

    /// No copy
    PcoThread& operator= ( const PcoThread & ) = delete;


    ///
    /// \brief joins on the thread.
    ///
    /// This method blocks the caller thread until the thread object finishes its
    /// execution.
    ///
    void join()
    {
        PcoManager::getInstance()->randomSleep(PcoManager::EventType::ThreadJoin);
        m_thread->join();
        PcoManager::getInstance()->randomSleep(PcoManager::EventType::ThreadJoin);
    }

    ///
    /// \brief sleeps for a certain number of microseconds
    /// \param useconds The number of microseconds to put the thread asleep
    ///
    static void usleep(uint64_t useconds);

    ///
    /// \brief requests the thread to stop
    ///
    /// This function simply modifies an internal boolean. The thread function has
    /// to check the existence of the request by calling stopRequested().
    ///
    void requestStop();

    ///
    /// \brief checks if there is a stop request
    /// \return true if there is a stop request, false else
    ///
    /// This function can be called from the function executed by the thread,
    /// thanks to the static function thisThread():
    ///
    /// PcoThread::thisThread()->stopRequested()
    ///
    bool stopRequested();

    ///
    /// \brief Exits the current thread
    ///
    /// This function exits the current thread. Obviously it should be a PcoThread
    /// to work as expected.
    static void exitThread();
    
    ///
    /// \brief Returns a pointer to the executing thread
    /// \return A pointer to the thread from which the function is called
    ///
    /// If called from outside of a PcoThread execution, returns nullptr.
    ///
    static PcoThread* thisThread();

protected:

    ///
    /// \brief gets the Id of the actual std::thread
    /// \return the Id of the actual std::thread
    ///
    std::thread::id getId();

    /// The Id of the actual std::thread
    std::thread::id m_id;

    /// The internal thread descriptor
    std::unique_ptr<std::thread> m_thread;

    /// Stores the presence of a stop request
    bool m_stopRequested{false};

    /// A mutex to protect the variable m_stopRequested
    std::unique_ptr<std::mutex> m_requestMutex;

    /// PcoManager is a friend, to simplify its development
    friend PcoManager;
};

#endif // PCOTHREAD_H
