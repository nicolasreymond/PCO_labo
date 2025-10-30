#ifndef PCOLOGGER_H
#define PCOLOGGER_H

#include <iostream>
#include <string.h>
#include <mutex>
#include <sstream>

///
/// \brief The PcoLogger class
///
/// This class can be used as std::cout, but will avoid any issue of interleaving
/// in multi-threaded applications.
///
/// It shall be used like this:
///
/// PcoLogger() << "Hi guys. Here is a number : " << i << std::endl;
///
class PcoLogger : public std::ostringstream
{
private:

    /// A mutex to protect the writing to std::cout
    static std::mutex sm_mutex;

public:

    /// Default constructor
    PcoLogger() = default;

    /// The descructor
    ///
    /// This method is where the writing to std::cout happens. That's the trick.
    ~PcoLogger()
    {
        std::lock_guard<std::mutex> guard(sm_mutex);
        if (sm_verbosity > 0) {
            std::cout << this->str();
        }
    }

    /// Sets the verbosity level
    static void setVerbosity(int level) {
        std::lock_guard<std::mutex> guard(sm_mutex);
        sm_verbosity = level;
    }

    ///
    /// \brief Initializes the PcoLogger
    /// \param argc the main program arguments
    /// \param argv the main program arguments
    ///
    /// It parses the main() arguments, and if it finds -verbose
    ///
    void initialize(int argc, char **argv) {
        if (argc == 2) {
            if (strcmp(argv[1], "-verbose") == 0) {
                setVerbosity(1);
            }
        }
    }

private:
    static int sm_verbosity;
};

// For retro-compabitility with Pco exercices
#define logger PcoLogger

#endif // PCOLOGGER_H
