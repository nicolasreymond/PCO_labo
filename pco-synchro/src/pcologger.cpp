#include "pcologger.h"

/// The static mutex of PcoLogger
std::mutex PcoLogger::sm_mutex;

int PcoLogger::sm_verbosity{0};
