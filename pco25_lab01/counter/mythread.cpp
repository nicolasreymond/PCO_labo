#include "mythread.h"

static volatile long unsigned int counter;

void runTask(unsigned long nbIterations)
{
    unsigned long local = 0;
    for (unsigned long i = 0; i < nbIterations; ++i) {
        local++;
    }
    counter += local;
}

void initCounter()
{
    counter = 0;
}

long unsigned int getCounter()
{
    return counter;
}
