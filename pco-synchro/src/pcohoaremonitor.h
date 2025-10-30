#ifndef PCOHOAREMONITOR_H
#define PCOHOAREMONITOR_H
#include "pcosemaphore.h"

///
/// \brief The PcoHoareMonitor class
///
/// This class allows to easily create a monitor as defined by Hoare in 1974.
/// It offers functions that allow to define the entry point of the monitor
/// as well as condition variables and the functions needed to use them.
/// It is meant to be a superclass of an actual implementation, as presented
/// in the example.
///
class PcoHoareMonitor
{
protected:

    /// A simple constructor
    PcoHoareMonitor();

    ///
    /// \brief The Condition class
    ///
    /// A condition represents a Hoare condition. It has to be used within
    /// the monitor, and serves as thread blocker and signaler.
    ///
    class Condition
    {
        /// The monitor is a friend to ease its development
        friend PcoHoareMonitor;

    public:

        /// A default constructor
        Condition() = default;

    private:

        /// An internal semaphore to block the threads
        PcoSemaphore waitingSem{0, false};

        /// A counter to know how many threads are blocked on the condition
        int nbWaiting{0};
    };

    ///
    /// \brief Function to enter into the monitor
    ///
    /// This function has to be called at the beginning of each function being
    /// an entry point to the monitor.
    ///
    void monitorIn();

    ///
    /// \brief Function to leave the monitor
    ///
    /// This function has to be called at the end of each function being
    /// an entry point to the monitor.
    ///
    void monitorOut();

    ///
    /// \brief Waits unconditionaly on a Condition
    /// \param cond The condition to wait on
    ///
    /// This function implements the waiting on a condition, as defined by Hoare.
    /// When the thread is waken by a signal, it continues with the mutual
    /// exclusion.
    ///
    void wait(Condition &cond);

    ///
    /// \brief Signals a condtion to potentially wake up a thread
    /// \param cond The condition to signal
    ///
    /// This function implements the signaling of a condition, as defined by
    /// Hoare. If no thread is waiting for the condition, then nothing happens,
    /// but if there is one the thread calling signal is suspended, waiting for
    /// the other one to finish.
    ///
    void signal(Condition &cond);

private:

    //! Mutex for the protecting the entire monitor
    PcoSemaphore monitorMutex;
    //! Blocking semaphore for the thread doing a signal(condition)
    PcoSemaphore monitorSignale;
    //! Number of threads waiting on the semaphore monitorSignale
    int monitorNbSignale;
};


#endif // PCOHOAREMONITOR_H
