#pragma once
#include <stdint.h>
//#include <sTune.h>
#include <QuickPID.h>
#ifndef PIDController_h
#define PIDController_h
extern float Setpoint, Input, Output;

class PIDController{

private:
// user settings
uint32_t settleTimeSec = 10;
uint32_t testTimeSec = 1000;  // 500 runPid interval = testTimeSec / samples
//const uint16_t samples = 500;
const uint16_t samples = 1000;
const float inputSpan = 200;

float outputStart = 0;
float outputStep = 750; // etais 50 (20 %)
float tempLimit = 100;
uint8_t debounce = 1;
int timer_interval_ms;
uint8_t ssr_relay_pin;

public:
PIDController(int Timer_interval_ms,uint8_t ssr_relay_pin);
void start_pid_tuning();
void setPID();
void automaticTuningloop();
void PIDloop();
float getCurrentPower();

//Define the aggressive and conservative and POn Tuning Parameters
double aggKp = 3.27 * 2.56, aggKi = 0.0008* 2.56, aggKd = 646.15* 2.56;
double consKp = 1.91* 2.56, consKi = 0.0004* 2.56, consKd = 6.282* 2.56;
float Kp = aggKp , Ki = aggKi, Kd = aggKd; //Kp = 11.0961 , Ki = 0.0124, Kd = 7.5; // Kp = 39.85 , Ki = 0.0021, Kd = 99.1;
bool automatictuning = false;

//sTune tuner;
//Specify the links
QuickPID myPID;
bool overridepid = false;
const float outputSpan = 1000;
bool pidtuningtest = false;


};


PIDController::PIDController(int Timer_interval_ms,uint8_t Ssr_relay_pin){
timer_interval_ms = Timer_interval_ms;
ssr_relay_pin = Ssr_relay_pin;
//tuner = sTune(&Input, &Output, tuner.NoOvershoot_PID, tuner.directIP, tuner.printOFF);

myPID = QuickPID(&Input, &Output, &Setpoint);

}

void PIDController::PIDloop(){
      
      debounce = 0;
      
      float gap = abs(Setpoint - Input); //distance away from setpoint
      if (gap < 4.5) { //we're close to setpoint, use conservative tuning parameters
        myPID.SetTunings(consKp, consKi, consKd);
      } else {
        //we're far from setpoint, use aggressive tuning parameters
        myPID.SetTunings(aggKp, aggKi, aggKd);
      }
      //otaio->sendmessage("Input before PID="+String(Input) + " Output " + String(Output));
      myPID.Compute();
      //otaio->sendmessage("Input after PID="+String(Input) + " Output " + String(Output));

}
float PIDController::getCurrentPower(){
    return Output / outputSpan;
}
void PIDController::start_pid_tuning(){
  /*tuner = sTune(&Input, &Output, tuner.NoOvershoot_PID, tuner.directIP, tuner.printOFF);
//Specify the links
  QuickPID myPID(&Input, &Output, &Setpoint);
  tuner.Configure(inputSpan, outputSpan, outputStart, outputStep, testTimeSec, settleTimeSec, samples);
  tuner.SetEmergencyStop(tempLimit);*/

}


void PIDController::setPID(){
 
 // myPID.SetOutputLimits(0, outputSpan * 0.8);
  myPID.Initialize();
  
  myPID.SetSampleTimeUs(timer_interval_ms * 1000);
  
 // debounce = 0; // ssr mode
  //Output = outputStep;
  myPID.SetMode(myPID.Control::automatic); // the PID is turned on
  
 // myPID.SetProportionalMode(myPID.pMode::pOnMeas);
  //myPID.SetAntiWindupMode(myPID.iAwMode::iAwClamp);
  myPID.SetTunings(aggKp, aggKi, aggKd); // update PID with the new tunings
  myPID.SetOutputLimits(0,160);
}

void PIDController::automaticTuningloop(){
// Not maintained !! May not work
/*float optimumOutput = tuner.softPwm(ssr_relay_pin, Input, Output, Setpoint, outputSpan, debounce);
    switch (tuner.Run()) {
    case tuner.sample: // active once per sample during test
      // sensors.requestTemperatures();
      // Input = sensors.getTempC(tempsens);
      tuner.plotter(Input, Output, Setpoint, 0.5f, 3); // output scale 0.5, plot every 3rd sample
      break;

    case tuner.tunings: // active just once when sTune is done
      tuner.GetAutoTunings(&Kp, &Ki, &Kd); // sketch variables updated by sTune
      myPID.SetOutputLimits(0, outputSpan * 0.8);
      myPID.SetSampleTimeUs((outputSpan - 1) * 1000);
      debounce = 0; // ssr mode
      Output = outputStep;
      myPID.SetMode(myPID.Control::automatic); // the PID is turned on
      myPID.SetProportionalMode(myPID.pMode::pOnMeas);
      myPID.SetAntiWindupMode(myPID.iAwMode::iAwClamp);
      myPID.SetTunings(Kp, Ki, Kd); // update PID with the new tunings
      break;

    case tuner.runPid: // active once per sample after tunings
      // sensors.requestTemperatures();
      // Input = sensors.getTempC(tempsens);
      myPID.Compute();
      tuner.plotter(Input, optimumOutput, Setpoint, 0.5f, 3);
      break;
    }*/
}
#endif // PIDController.h