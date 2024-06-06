#pragma once
#include <stdint.h>
#ifndef PwmTimer_h
#define PwmTimer_h
// TIMER _______________________________________
// These define's must be placed at the beginning before #include "ESP8266TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

// Select a Timer Clock
#define USING_TIM_DIV1                false           // for shortest and most accurate timer
#define USING_TIM_DIV16               false           // for medium time and medium accurate timer
#define USING_TIM_DIV256              true            // for longest timer but least accurate. Default
#include "ESP8266TimerInterrupt.h"
class PwmTimer
{
private:
    ESP8266Timer ITimer;
    float& Output;
    int32_t Timerinterval;
    int8_t Ssrpin;
    void TimerHandler();
    static void TimerHandlerStatic();
    static PwmTimer* instance; 
    bool toggle;
public:
    PwmTimer(float& output,int32_t timerinterval,int8_t ssrpin):
    Output(output),Timerinterval(timerinterval), Ssrpin(ssrpin){
        // Init ESP8266 timer 1/*
        this->ITimer = ESP8266Timer();
        this->toggle = false;
    };
    
    void disableTimer();
    void startTimer();
    ~PwmTimer();
};


#endif // PwmTimer.h