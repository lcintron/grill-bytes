#include "elapsedtimecheck.h"


ElapsedTimeCheck::ElapsedTimeCheck(unsigned long updateInterval){
    _updateInterval = updateInterval;
    _isActive = false;
    _millisRef =0UL;
    _millisCount = 0UL;
}

bool ElapsedTimeCheck::isTimeElapsed(){
    
    updateTimeElapsed();
    return _millisCount > _updateInterval;
}


void ElapsedTimeCheck::updateTimeElapsed(){
    if(_isActive){
        unsigned long newMillis = millis();
        unsigned long millisDiff = newMillis - _millisRef;
        _millisRef = newMillis;
        _millisCount+=millisDiff;
    }
}

void ElapsedTimeCheck::start(){
    _millisRef = millis();
    _isActive = true;
}

void ElapsedTimeCheck::reset(){
    _millisRef = millis();
    _millisCount = 0UL;
}

void ElapsedTimeCheck::stop(){
    _isActive = false;
}

