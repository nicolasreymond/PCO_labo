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

#ifndef PCOCOMMON_H
#define PCOCOMMON_H

#include <map>
#include <mutex>
#include <thread>
#include <vector>


class PcoThread;
class PcoMutex;
class PcoSemaphore;
class PcoConditionVariable;

///
/// \brief The PcoWatchDog class
///
/// This abstract class has to be derived and set to the PcoManager.
/// Then the trigger function is called whenever a thread blocks on
/// a condition variable or a semaphore.
class PcoWatchDog
{
public:

    /// Empty virtual destructor
    virtual ~PcoWatchDog() = default;

    ///
    /// \brief trigger
    /// \param nbBlocked Number of thread blocked on a synchronization primitive.
    ///
    /// nbBlocked represents the number of threads blocked on any
    /// PcoSemaphore or PcoConditionVariable monitored.
    ///
    virtual void trigger(int nbBlocked) = 0;
};

///
/// \brief The PcoManager class
///
/// This class allows to inject some latency within the synchronization objects
///
class PcoManager
{
public:

    /// An enum to define the type of events on which some actions are executed
    enum class EventType {
        ThreadCreation,         ///< For the thread constructor function
        ThreadJoin,             ///< For the thread join() function
        MutexLock,              ///< For the mutex lock() function
        MutexUnlock,            ///< For the mutex unlock() function
        WaitConditionWait,      ///< For the condition variable wait() function
        WaitConditionNotify,    ///< For the condition variable notify() function
        WaitConditionNotifyAll, ///< For the condition variable notifyAll() function
        SemaphoreAcquire,       ///< For the semaphore acquire() function
        SemaphoreRelease,       ///< For the semaphore release() function
        Standard                ///< For any object, the default value
    };

    ///
    /// \brief gets the PcoManager singleton instance
    /// \return A pointer to the unique instance.
    ///
    /// The object is created the first time this method is called.
    ///
    static PcoManager *getInstance();

    ///
    /// \brief sets the maximum sleeping time for a specific event type
    /// \param useconds The number of microseconds to sleep
    /// \param eventType The event type
    ///
    /// By default it defines the sleeping time of the event EventType::Standard.
    ///
    /// The method randomSleep() uses the value defined for the specific event type
    /// it gets as an argument, but if this value is not set, then it uses the
    /// value defined for EventType::Standard
    ///
    void setMaxSleepDuration(unsigned int useconds, EventType eventType = EventType::Standard);

    ///
    /// \brief Let the calling thread sleeps for a random period
    /// \param eventType The event type used to get the correct maximum time
    ///
    /// This method is called by the various synchronization objects, letting
    /// them pass the kind of event, depending on their class.
    ///
    void randomSleep(EventType eventType = EventType::Standard);

    ///
    /// \brief gets a pointer to the PcoThread executing the call
    /// \return A pointer to the current PcoThread
    ///
    PcoThread* thisThread();

    ///
    /// \brief nbBlockedThreads
    /// \return The number of threads in a blocked state
    ///
    /// This value represents the number of threads that are blocked on:
    /// - PcoMutex::lock()
    /// - PcoSemaphore::acquire()
    /// - PcoConditionVariable::wait()
    ///
    int nbBlockedThreads();

    ///
    /// \brief setWatchDog
    /// \param watchDog A watchdog object that will be notified whenever necessary
    ///
    /// The watchdog will be notified whenever a thread blocks on a
    /// semaphore or a condition variable.
    ///
    void setWatchDog(PcoWatchDog *watchDog);

    ///
    /// \brief The Mode of execution for semaphores
    ///
    /// A Normal mode means that the registered semaphores behave as expected.
    /// In Free mode, the semaphore acquire() method is non-blocking.
    /// It is meant to be used in very specific contexts where at some stage
    /// the existing semaphores should be freed.
    ///
    /// The registered semaphores are the ones created with the monitoring set to true.
    /// See the PcoSemaphore() constructor for more details.
    ///
    enum class Mode {
        /// Normal mode of execution of a semaphore
        Normal,
        /// Free mode, where registered semaphores are non-blocking
        Free
    };

    ///
    /// \brief sets the semaphore mode to NormalMode
    ///
    void setNormalMode();

    ///
    /// \brief sets the semaphore mode to FreeMode
    ///
    /// When this function is called, all registered semaphores are released, so
    /// that all threads blocked on them are freed. Then the function acquire() is
    /// non-blocking.
    ///
    void setFreeMode();

    ///
    /// \brief gets the sempahore mode
    /// \return The current semaphore mode
    ///
    Mode getMode();

protected:

    ///
    /// \brief PcoManager constructor
    ///
    /// The constructor is protected, as it is meant to be a singleton.
    ///
    PcoManager();

    /// A destructors freeing memory and releasing resources
    ~PcoManager();

    ///
    /// \brief registers the PcoThread in the PcoManager
    /// \param thread The thread to register
    ///
    /// This function has to be called within the newly created thread.
    /// It updates the internal map of running threads
    ///
    void registerThread(PcoThread *thread);

    ///
    /// \brief unregisters the PcoThread in the PcoManager
    /// \param thread The thread to unregister
    ///
    /// This function has to be called within the thread function,
    /// just before leaving. It updates the internal map of running threads
    ///
    void unregisterThread(PcoThread *thread);

    ///
    /// \brief addWaitingThread
    ///
    /// Increments the number of waiting threads. To be called just before the
    /// threads goes to blocked status.
    ///
    void addWaitingThread();

    ///
    /// \brief removeWaitingThread
    ///
    /// Decrements the number of waiting threads. To be called just after the thread
    /// has been waken up.
    void removeWaitingThread();

    /// Map of sleeping times per type of event
    std::map<EventType, unsigned int> m_usecondsMap;

    /// Map of running threads
    std::map<std::thread::id, PcoThread *> m_runningThreads;

    /// Mutex to protect m_runningThreads
    std::recursive_mutex m_mutex;

    /// Mutex to protect the sleeping part of the methods
    std::mutex m_sleepMutex;

    /// Number of threads currently in a blocked state
    int m_nbBlockedThreads{0};

    /// A watchdog called when a thread blocks on a synchronization object
    PcoWatchDog *m_watchDog{nullptr};

    /// A vector of PcoSemaphore that are monitored to detect deadlocks
    std::vector<PcoSemaphore *> m_semaphores;

    ///
    /// \brief registers a semaphore to be used as a free one in case
    /// \param semaphore The semaphore to register
    ///
    /// This function should be called only once a semaphore. It does not
    /// check for duplicates, but that could be changed in the future.
    ///
    void registerSemaphore(PcoSemaphore *semaphore);

    ///
    /// \brief unregisters a semaphore
    /// \param semaphore The semaphore to unregister
    ///
    /// The semaphore has to be already registered thanks to registerSemaphore().
    /// However if the semaphore is not in the registered list the function
    /// will end up without warnings.
    ///
    void unregisterSemaphore(PcoSemaphore *semaphore);

    /// \brief the execution mode of semaphores
    /// By default, we use the normal mode to have a coherent behavior
    Mode m_mode{Mode::Normal};


    /// PcoThread is a friend just to help
    friend PcoThread;

    /// PcoMutex is a friend just to help
    friend PcoMutex;

    /// PcoSemaphore is a friend just to help
    friend PcoSemaphore;

    /// PcoConditionVariable is a friend just to help
    friend PcoConditionVariable;

};


#endif // PCOCOMMON_H
