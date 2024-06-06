#pragma once

#ifndef OTAInputOutput_h
#define OTAInputOutput_h
#include <stdint.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "PIDController.h"
class OTAInputOutput
{
private:
    PIDController pidctl;
    const char* Ssid;
    const char* Password;
    const char* Remoteipaddress;
    unsigned int RemotePort;
    WiFiUDP udp;
    IPAddress remoteIP;
    

public:
    OTAInputOutput();
    OTAInputOutput(PIDController pidctl, const char* ssid,const char* password,const char* remoteipaddress, unsigned int remoteport):
    pidctl(pidctl),Ssid(ssid),Password(password),Remoteipaddress(remoteipaddress),RemotePort(remoteport){
        remoteIP.fromString(Remoteipaddress);
    }
    void sendmessage(String message);
    void initwifiandudp();
    void getWifiPacket();
    ~OTAInputOutput();
};


void OTAInputOutput::sendmessage(String message){
  Serial.println(message);
  udp.beginPacket(remoteIP, RemotePort);
  udp.write(message.c_str()); // Convert String to char array
  udp.endPacket();
}
void OTAInputOutput::initwifiandudp(){
    WiFi.mode(WIFI_STA);
  WiFi.begin(Ssid, Password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!");
    return;
  }
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  udp.begin(RemotePort);
  sendmessage("Connecting");
}
void OTAInputOutput::getWifiPacket(){
  int packetSize = udp.parsePacket();
  if(!packetSize) return;
  char buffer[packetSize];
  int len = udp.read(buffer, packetSize);
  String message = String(buffer);
  message = message.substring(0, len);
  if(message.indexOf("OverridePID")!=-1){
    pidctl.overridepid = true;
    return;
  }
  if(message.indexOf("ExternalPIDTest")!=-1){
    pidctl.pidtuningtest = true;
    return;
  }


  /*if(message.indexOf("EnablePIDandTest")!=-1){
    pidctl.overridepid = false;
    start_pid_tuning();
    return;
  }*/
  if(message.indexOf("SetSP")!=-1){
      String parameters = message.substring(6, len-1);
      Serial.println(parameters);
      Setpoint = parameters.toFloat();
      pidctl.myPID.Initialize();
      sendmessage("New SetPoint : " + String(Setpoint));
      return;
  }
  if(message.indexOf("SetTunings")!=-1){
    String parameters = message.substring(10, len);
    String tuningtype = "";
    if(parameters.indexOf("agg")!=-1){
      tuningtype = "agg";
      parameters = parameters.substring(4, len-2);
    }else if(parameters.indexOf("cons")!=-1){
      tuningtype = "cons";
      parameters = parameters.substring(5, len-2);
    }
      Serial.println("New Aggressive PID Settings: " + parameters);
      int firstcomma = parameters.indexOf(",");
      //(Kp,Ki,Kd)
      float Kp = parameters.substring(0, firstcomma).toFloat();
      int secondcomma = parameters.indexOf(",",firstcomma+1);
      float Ki = parameters.substring(firstcomma+1, secondcomma).toFloat();
      float Kd = parameters.substring(secondcomma+1, parameters.length()).toFloat();
      if(tuningtype == "agg"){
        pidctl.aggKp = Kp;
        pidctl.aggKi = Ki;
        pidctl.aggKd = Kd;
        
      }else if(tuningtype == "cons"){
        pidctl.consKp = Kp;
        pidctl.consKi = Ki;
        pidctl.consKd = Kd;
      }
      sendmessage("New PID values: " + tuningtype + "Kp="+ String(Kp) + "Ki="+ String(Ki) +"Kd=" + String(Kd));
    }
  Serial.println("Received message: " + message);
    
}
OTAInputOutput::~OTAInputOutput()
{
}

#endif // OTAInputOutput.h