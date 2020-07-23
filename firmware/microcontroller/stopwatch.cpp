#include "stopwatch.h""


StopWatch::StopWatch()
{
    _isActive = false;
    _millisRef =0;
    _millisCount = 0;
    _seconds = 0;
    _minutes=0;
    _hours = 0;
}

void StopWatch::toggle()
{
    if(_isActive)
    {
        _isActive = false;
    }else
    {
        _isActive = true;
    }
    _millisRef = millis();
}

void StopWatch::reset()
{

    _isActive=false;
    _millisRef = 0;
    _millisCount = 0;
    _seconds = 0;
    _minutes=0;
    _hours = 0;
}

bool StopWatch::isActive()
{
    
    return _isActive;
}

void StopWatch::tick()
{
    if(_isActive){
        unsigned long newMillis = millis();
        unsigned long millisDiff = newMillis - _millisRef;
        _millisRef = newMillis;
        _millisCount+=millisDiff;
        calculateTime();
    }
}

void StopWatch::calculateTime()
{
    _seconds = (unsigned int) (_millisCount / 1000UL) % 60UL ;
    _minutes = (unsigned int) ((_millisCount / (1000UL*60UL)) % 60UL);
    _hours   = (unsigned int) ((_millisCount / (1000UL*60UL*60UL)) % 24UL);
    if(_hours>99)
    {
        reset();
    }
}

String StopWatch::toString()
{
    char data[8];
    sprintf(data, "%02d:%02d:%02d", _hours, _minutes, _seconds);
    return data;
}