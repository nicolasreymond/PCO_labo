// Authors: Nicolas Reymond, Nadia Cattin

#include "primenumberdetector.h"

static PcoMutex mutex;

bool PrimeNumberDetector::isPrime(uint64_t number)
{
    if (number < 2 || number % 2 == 0)
    {
        return false;
    }
    const uint64_t maxDivisor = static_cast<uint64_t>(std::sqrt(number));

    for (uint64_t i = 3; i <= maxDivisor; i += 2)
    {
        if (number % i == 0)
        {
            return false;
        }
    }

    return true;
}

PrimeNumberDetectorMultiThread::PrimeNumberDetectorMultiThread(size_t nbThreads)
    : nbThreads(nbThreads)
{
}

bool PrimeNumberDetectorMultiThread::isPrime(uint64_t number)
{
    if (number < 2 || number % 2 == 0)
    {
        return false;
    }

    const uint64_t maxDivisor = static_cast<uint64_t>(std::sqrt(number));
    const uint64_t step = maxDivisor / nbThreads;
    bool divisorFound = false;

    std::vector<std::unique_ptr<PcoThread>> threads;
    threads.reserve(nbThreads);

    for (uint32_t i = 0; i < nbThreads; ++i)
    {
        const uint64_t start = 3 + (i * step);
        const uint64_t end = (i == nbThreads - 1) ? maxDivisor : start + step - 1;

        threads.push_back(std::unique_ptr<PcoThread>(
            new PcoThread(testRange, number, start, end, std::ref(divisorFound))));
    }

    for (std::unique_ptr<PcoThread> &thread : threads)
    {
        thread->join();
    }

    return !divisorFound;
}

void PrimeNumberDetectorMultiThread::testRange(uint64_t n, uint64_t start, uint64_t end, bool &divisorFound)
{
    if (start % 2 == 0)
    {
        start++;
    }
    int counter = 0;
    for (uint64_t i = start; i <= end; i += 2)
    {
        if (n % i == 0)
        {
            mutex.lock();
            divisorFound = true;
            mutex.unlock();
            return;
        }
        counter++;
        if (counter % checkInterval == 0)
        {
            mutex.lock();
            if (divisorFound)
            {
                mutex.unlock();
                return;
            }
            mutex.unlock();
        }
    }
}