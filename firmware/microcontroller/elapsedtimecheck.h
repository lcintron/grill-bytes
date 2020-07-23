#ifndef __ELAPSEDTIMECHECK_H__
#define __ELAPSEDTIMECHECK_H__
#endif
#include <Particle.h>

class ElapsedTimeCheck
{
    public:
        ElapsedTimeCheck(unsigned long updateInterval);
        
        bool isTimeElapsed();
        void start();
        void reset();
        void stop();
        
    private:
        unsigned long _updateInterval;
        unsigned long _millisRef;
        unsigned long _millisCount;
        bool _isActive;
        void updateTimeElapsed();
};


