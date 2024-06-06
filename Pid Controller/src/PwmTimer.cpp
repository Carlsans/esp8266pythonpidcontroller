#include "PwmTimer.h"

PwmTimer* PwmTimer::instance = nullptr;
void PwmTimer::TimerHandler()
{
  static uint32_t interval;
  if(this->toggle){
    interval =(int32_t) (this->Timerinterval * this->Output/255 )+1;
  }else{
    interval =(int32_t) (this->Timerinterval * (1 - this->Output/255))+1;
  }
  PwmTimer::instance = this;
  
  this->ITimer.setInterval( interval * 1000 , PwmTimer::TimerHandlerStatic);

  digitalWrite(this->Ssrpin, this->toggle);
  this->toggle = !this->toggle;
}
void PwmTimer::TimerHandlerStatic()
{
    if (PwmTimer::instance) {
        PwmTimer::instance->TimerHandler();
    }
}

void PwmTimer::startTimer(){
    // Timer
  // Interval in microsecs
 PwmTimer::instance = this;
 if (ITimer.attachInterruptInterval(this->Timerinterval * 1000, PwmTimer::TimerHandlerStatic))
  {
    Serial.print(F("Starting  ITimer OK, millis() = "));
    Serial.println(millis());
    //sendmessage("Starting  ITimer"+String(millis()));
  }
  else{
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
    //sendmessage("Can't set ITimer. Select another freq. or timer");
    }
}
void PwmTimer::disableTimer(){
    this->ITimer.disableTimer();
}



PwmTimer::~PwmTimer()
{
}
