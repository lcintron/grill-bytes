#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__
#endif
#include <Particle.h>

class StopWatch
{
    public:
        StopWatch();
        
        void toggle();
        void reset();
        bool isActive();    
        void tick();
        unsigned int getSeconds();
        unsigned int getMinutes();
        unsigned int getHours();
        String toString();
    
    private:
        unsigned long _millisRef;
        unsigned long _millisCount;
        bool _isActive;
        unsigned int _seconds;
        unsigned int _minutes;
        unsigned int _hours;
    
        void calculateTime();
};


