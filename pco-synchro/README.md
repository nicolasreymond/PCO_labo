# PCOSynchro library

PcoSynchro is a library of classes implementing the classical synchronization mechanisms. It offers:

- PcoThread
- PcoMutex
- PcoSemaphore
- PcoConditionVariable

There classes are wrappers around the objects found in the standard library, offering a subset of the functionalities. Why a subset? Because we use it within a concurrent programming course, and we want to restrict some usage to guide the students.

Moreover, the library offers a mechanism to add random sleeps in the various methods, allowing to test more deeply some synchronization mechanisms such as the producer-consumer or the reader-writer protocols. A class PcoManager can be used to set specific ranges for the random sleeps for all classes or for each specific method.

The library is open source, with a LGPL license.

To compile, use cmake:

    cd lib/pcosynchro
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    
or qmake:

    cd lib/pcosynchro
    mkdir build
    cd build
    qmake ..
    make
    sudo make install

Some tests based on GoogleTest are implemented, so as to test the correct behavior of the various classes, and can serve as examples of a way to create tests checking for deadlocks for instance.

The tests can be compiled with cmake and run:

    cd test
    mkdir build
    cd build
    cmake ..
    make
    ./pcosynchrotest
    
or with qmake:

    cd test
    mkdir build
    cd build
    qmake ..
    make
    ./pcosynchrotest


Author: Yann Thoma
