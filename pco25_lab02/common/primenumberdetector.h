// Authors: Nicolas Reymond, Nadia Cattin

#ifndef PRIMENUMBERDETECTOR_H
#define PRIMENUMBERDETECTOR_H

#include "logging.h"

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcomutex.h>

/**
 * @brief Interface for prime number detection algorithms
 */
class PrimeNumberDetectorInterface
{
public:
    virtual ~PrimeNumberDetectorInterface() = default;

    /**
     * @brief Check if a given number is prime
     * @param number The number to check
     * @return true if the number is prime, false otherwise
     */
    virtual bool isPrime(uint64_t number) = 0;
};

/**
 * @brief Single-threaded prime number detector
 */
class PrimeNumberDetector : public PrimeNumberDetectorInterface
{
public:
    bool isPrime(uint64_t number) override;
};

/**
 * @brief Multi-threaded prime number detector
 */
class PrimeNumberDetectorMultiThread : public PrimeNumberDetectorInterface
{
public:
    /**
     * @brief Construct a multi-threaded prime detector
     * @param nbThreads Number of threads to use for detection
     */
    explicit PrimeNumberDetectorMultiThread(size_t nbThreads);

    bool isPrime(uint64_t number) override;

private:
    static const uint64_t checkInterval = 1000;
    size_t nbThreads;
    static void testRange(uint64_t number, uint64_t lower, uint64_t upper, bool &divisorFound);
};

#endif // PRIMENUMBERDETECTOR_H
