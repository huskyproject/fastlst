#include <windows.h>

            // implementation in mutex.cpp

// Class to handle named Mutex semaphore

#define     MUTEX_OK        0
#define     MUTEX_TIMEOUT   1
#define     MUTEX_ERROR    -1


class Mutex {
    private:
        HANDLE mutexh;
    public:
        Mutex( const char *name ); // semaphore name
        int Wait ( DWORD msec);  // 0 = test only, INFINITE = no timeout
        void Release ();        // not necessary
        ~Mutex ();
};



