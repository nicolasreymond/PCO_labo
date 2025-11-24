//  /$$$$$$$   /$$$$$$   /$$$$$$         /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$ 
// | $$__  $$ /$$__  $$ /$$__  $$       /$$__  $$ /$$$_  $$ /$$__  $$| $$____/ 
// | $$  \ $$| $$  \__/| $$  \ $$      |__/  \ $$| $$$$\ $$|__/  \ $$| $$      
// | $$$$$$$/| $$      | $$  | $$        /$$$$$$/| $$ $$ $$  /$$$$$$/| $$$$$$$ 
// | $$____/ | $$      | $$  | $$       /$$____/ | $$\ $$$$ /$$____/ |_____  $$
// | $$      | $$    $$| $$  | $$      | $$      | $$ \ $$$| $$       /$$  \ $$
// | $$      |  $$$$$$/|  $$$$$$/      | $$$$$$$$|  $$$$$$/| $$$$$$$$|  $$$$$$/
// |__/       \______/  \______/       |________/ \______/ |________/ \______/ 

#include <gtest/gtest.h>
#include <atomic>
#include <vector>
#include <memory>

#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcosemaphore.h>

#include "sharedsection.h"
#include "sharedsectioninterface.h"

static void enterCritical(std::atomic<int>& nbIn) {
    int now = nbIn.fetch_add(1) + 1;
    ASSERT_EQ(now, 1) << "Deux locomotives dans la section en même temps !";
}
static void leaveCritical(std::atomic<int>& nbIn) {
    nbIn.fetch_sub(1);
}

// Test fixtures (sections)
class SharedSectionErrors : public ::testing::Test {};
class SharedSectionSerialization : public ::testing::Test {};
class SharedSectionRobustness : public ::testing::Test {};


// ============================================================================
// SharedSectionErrors Tests
// ============================================================================

TEST_F(SharedSectionErrors, NbErrors_ZeroInitially) {
    SharedSection section;
    ASSERT_EQ(section.nbErrors(), 0);
}

TEST_F(SharedSectionErrors, DirectionsAreDistinct) {
    using Direction = SharedSectionInterface::Direction;
    ASSERT_NE(Direction::D1, Direction::D2);
}

TEST_F(SharedSectionErrors, BasicSequence_NoErrors) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 0);
}

TEST_F(SharedSectionErrors, SequentialDifferentLocos_NoErrors) {
    SharedSection section;
    Locomotive l1(1, 10, 0), l2(2, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);

    section.access(l2, SharedSectionInterface::Direction::D1);
    section.leave(l2, SharedSectionInterface::Direction::D1);
    section.release(l2);

    ASSERT_EQ(section.nbErrors(), 0);
}

TEST_F(SharedSectionErrors, RepeatedCorrectSequence_NoError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    for (int i = 0; i < 50; ++i) {
        section.access(l1, SharedSectionInterface::Direction::D1);
        section.leave(l1, SharedSectionInterface::Direction::D1);
        section.release(l1);
    }

    ASSERT_EQ(section.nbErrors(), 0);
}

TEST_F(SharedSectionErrors, LeaveWithoutAccess_IsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.leave(l1, SharedSectionInterface::Direction::D1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST_F(SharedSectionErrors, ReleaseWithoutLeave_IsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST_F(SharedSectionErrors, ReleaseWithoutAccess_Increments) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.release(l1);
    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 2);
}

TEST_F(SharedSectionErrors, ConsecutiveAccess_IsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST_F(SharedSectionErrors, DoubleLeave_IncrementsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST_F(SharedSectionErrors, LeaveWrongDirection_IsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D2);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST_F(SharedSectionErrors, LeaveAfterRelease_IsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);
    section.leave(l1, SharedSectionInterface::Direction::D1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST_F(SharedSectionErrors, AccessMultipleTimesWithoutRelease_IsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);
    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST_F(SharedSectionErrors, WrongRelease_DifferentLocomotive_IsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0), l2(2, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l2);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST_F(SharedSectionErrors, MultipleErrorsDetected) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D2);
    section.release(l1);

    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 6);
}


// ============================================================================
// SharedSectionSerialization Tests
// ============================================================================

TEST_F(SharedSectionSerialization, TwoSameDirection_SerializesCorrectly) {
    SharedSection section;
    std::atomic<int> nbIn{0};
    Locomotive l1(1, 10, 0), l2(2, 10, 0);

    PcoThread t1([&]{
        section.access(l1, SharedSectionInterface::Direction::D1);
        enterCritical(nbIn);
        PcoThread::usleep(1000);
        leaveCritical(nbIn);
        section.leave(l1, SharedSectionInterface::Direction::D1);
        section.release(l1);
    });

    PcoThread t2([&]{
        PcoThread::usleep(500);
        section.access(l2, SharedSectionInterface::Direction::D1);
        enterCritical(nbIn);
        leaveCritical(nbIn);
        section.leave(l2, SharedSectionInterface::Direction::D1);
    });

    t1.join(); t2.join();
    ASSERT_EQ(section.nbErrors(), 0);
}

TEST_F(SharedSectionSerialization, TwoOppositeDirection_SerializesCorrectly) {
    SharedSection section;
    std::atomic<int> nbIn{0};
    Locomotive l1(1, 10, 0), l2(2, 10, 0);

    PcoThread t1([&]{
        section.access(l1, SharedSectionInterface::Direction::D1);
        enterCritical(nbIn);
        PcoThread::usleep(1000);
        leaveCritical(nbIn);
        section.leave(l1, SharedSectionInterface::Direction::D1);
        section.release(l1);
    });

    PcoThread t2([&]{
        PcoThread::usleep(500);
        section.access(l2, SharedSectionInterface::Direction::D2);
        enterCritical(nbIn);
        leaveCritical(nbIn);
        section.leave(l2, SharedSectionInterface::Direction::D2);
        section.release(l2);
    });

    t1.join(); t2.join();
    ASSERT_EQ(section.nbErrors(), 0);
}

TEST_F(SharedSectionSerialization, MultipleLocomotives_SerializesCorrectly) {
    SharedSection section;
    std::atomic<int> nbIn{0};
    const int nLocos = 10;
    std::vector<std::unique_ptr<Locomotive>> locos;
    std::vector<std::unique_ptr<PcoThread>> threads;

    locos.reserve(nLocos);
    threads.reserve(nLocos);

    for (int i = 0; i < nLocos; ++i) {
        locos.emplace_back(std::make_unique<Locomotive>(i + 1, 10, 0));
    }

    for (int i = 0; i < nLocos; ++i) {
        threads.emplace_back(std::make_unique<PcoThread>([&, i]{
            section.access(*locos[i], SharedSectionInterface::Direction::D1);
            enterCritical(nbIn);
            PcoThread::usleep(100);
            leaveCritical(nbIn);
            section.leave(*locos[i], SharedSectionInterface::Direction::D1);
            section.release(*locos[i]);
        }));
    }

    for (auto& tptr : threads) {
        tptr->join();
    }

    ASSERT_EQ(section.nbErrors(), 0);
}


// ============================================================================
// SharedSectionRobustness Tests
// ============================================================================

TEST_F(SharedSectionRobustness, ManySequentialAccesses_NoErrors) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    for (int i = 0; i < 200; ++i) {
        auto dir = (i % 2 == 0) ? SharedSectionInterface::Direction::D1 : SharedSectionInterface::Direction::D2;
        section.access(l1, dir);
        section.leave(l1, dir);
        section.release(l1);
    }

    ASSERT_EQ(section.nbErrors(), 0);
}

TEST_F(SharedSectionRobustness, StopAll_Idempotent) {
    SharedSection section;
    section.stopAll();
    section.stopAll();
    SUCCEED();
}

TEST_F(SharedSectionRobustness, AccessAfterLeave_Works) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);

    section.access(l1, SharedSectionInterface::Direction::D2);
    section.leave(l1, SharedSectionInterface::Direction::D2);
    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 0);
}

TEST_F(SharedSectionRobustness, ReaccessAfterRelease_Works) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);

    section.access(l1, SharedSectionInterface::Direction::D2);
    section.leave(l1, SharedSectionInterface::Direction::D2);
    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 0);
}

TEST_F(SharedSectionRobustness, ManyReleases_IncrementsErrorCount) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    const int n = 7;
    for (int i = 0; i < n; ++i) {
        section.release(l1);
    }

    ASSERT_EQ(section.nbErrors(), n);
}

TEST_F(SharedSectionRobustness, StopAll_DoesNotCrash) {
    SharedSection section;
    const int nLocos = 8;
    std::vector<std::unique_ptr<Locomotive>> locos;
    std::vector<std::unique_ptr<PcoThread>> threads;
    locos.reserve(nLocos);
    threads.reserve(nLocos);

    for (int i = 0; i < nLocos; ++i) {
        locos.emplace_back(std::make_unique<Locomotive>(i + 1, 10, 0));
    }

    for (int i = 0; i < nLocos; ++i) {
        threads.emplace_back(std::make_unique<PcoThread>([&, i]{
            PcoThread::usleep(100 + (i * 10));
            section.access(*locos[i], SharedSectionInterface::Direction::D1);
            PcoThread::usleep(20);
            section.leave(*locos[i], SharedSectionInterface::Direction::D1);
            section.release(*locos[i]);
        }));
    }

    section.stopAll();

    for (auto& tptr : threads) tptr->join();

    SUCCEED();
}

TEST_F(SharedSectionRobustness, NbErrors_IsThreadSafe) {
    SharedSection section;
    const int nThreads = 5;
    const int nIterations = 1000;
    std::vector<std::unique_ptr<PcoThread>> threads;
    threads.reserve(nThreads);

    for (int i = 0; i < nThreads; ++i) {
        threads.emplace_back(std::make_unique<PcoThread>([&]{
            for (int j = 0; j < nIterations; ++j) {
                Locomotive tmp(999, 10, 0);
                section.release(tmp);
            }
        }));
    }

    for (auto& tptr : threads) tptr->join();

    ASSERT_EQ(section.nbErrors(), nThreads * nIterations);
}
