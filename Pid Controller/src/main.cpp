#include <Arduino.h>
#include <ArduinoOTA.h>
// WEB SERIAL __________________________________
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DallasTemperature.h>
#include <PwmTimer.h>
#include <OTAInputOutput.h>
#include <PIDController.h>
extern float Setpoint, Input, Output;
float Setpoint =38;
float Input = 0;
float Output = 0;
#define TIMER_INTERVAL_MS        3000
const char* ssid = "yourssid";          // Your WiFi SSID
const char* password = "yourpassword";  // Your WiFi Password
//ex : const char* remoteipaddress = "192.168.2.33";
const char* remoteipaddress = "yourip";
unsigned int remotePort = 5000; 

#define SSR_RELAY_PIN 16
#define TEMP_SENSOR_PIN 12
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress tempsens;
PIDController pidctl(TIMER_INTERVAL_MS,SSR_RELAY_PIN);
OTAInputOutput otaio = OTAInputOutput(pidctl,ssid,password,remoteipaddress,remotePort);
PwmTimer pwmTimer(Output,TIMER_INTERVAL_MS,SSR_RELAY_PIN);

bool isotaupdating = false;



uint32_t lastsensorreadingmillis = 0;
int32_t readSensor(){
  int32_t starttime = millis();
  sensors.requestTemperatures();
  Input = sensors.getTempC(tempsens);
  return millis() - starttime;
}

void setup_sensors(){
sensors.begin();
  //Serial.println(sensors.getDeviceCount(), DEC);
  if (!sensors.getAddress(tempsens, 0)) Serial.println("Unable to find address for Device 0");
  sensors.setResolution(tempsens, 10); // was 9
  readSensor();
  Serial.println(Input);
  
}


void setup()
{
 ArduinoOTA.onStart([]() {
    isotaupdating = true;
    pwmTimer.disableTimer();
  });
 ArduinoOTA.begin();
 
 Serial.begin(9600,SERIAL_8N1);
 
 otaio.initwifiandudp();
  pinMode(SSR_RELAY_PIN, OUTPUT);
 
  setup_sensors();
 // if(pidctl.automatictuning) 
  //  pidctl.start_pid_tuning();
  //else
  pidctl.setPID();
  
  pwmTimer.startTimer();
  
}


void loop()
{
  ArduinoOTA.handle();
  
  if (lastsensorreadingmillis < millis() + TIMER_INTERVAL_MS - 100){
    lastsensorreadingmillis = millis();
    otaio.sendmessage("Sensor reading time = " + String(readSensor())); 
  }
  
  
  if (pidctl.overridepid){
    if(Input > Setpoint){
      digitalWrite(SSR_RELAY_PIN, LOW);
    }else{
      digitalWrite(SSR_RELAY_PIN, HIGH);
      Output = pidctl.outputSpan ;
    }
    
  }else if (pidctl.automatictuning){
    pidctl.automaticTuningloop();

    }else if (!pidctl.overridepid){
      pidctl.PIDloop();
    }

 
  otaio.sendmessage("SetPoint: "+String(Setpoint));
  otaio.sendmessage("PIDpower: "+String(pidctl.getCurrentPower()));
  otaio.sendmessage("Sensor temp: "+String(Input));           
  otaio.sendmessage("Output: "+String(Output));
  otaio.sendmessage("Override PID: "+ String(pidctl.overridepid));
  
  otaio.getWifiPacket();
  
  if(isnan(Output))
    ESP.restart();
  otaio.sendmessage("PIDsettings(Kp,Ki,Kd): " + String(pidctl.myPID.GetKp())+ "," + String(pidctl.myPID.GetKi())+ "," + String(pidctl.myPID.GetKd()));


  
  
}

  
  


