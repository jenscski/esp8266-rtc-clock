#include <Arduino.h>
#include <ArduinoOTA.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Schedule.h>

#include <TaskSchedulerDeclarations.h>
#include <Timezone.h>
#include <Wire.h>
#include <RtcDS3231.h>

#ifndef _MAIN_H_
#define _MAIN_H_

// Scheduler
extern Scheduler m_Scheduler;
    
extern WiFiUDP m_NtpUdp;

extern Timezone m_Timezone;

extern RtcDS3231<TwoWire> m_Rtc;

void cbNtpUpdate();
bool cbNtpUpdateEnable();
void cbNtpUpdateDisable();

void cbOtaOnStart();
void cbOtaOnEnd();
void cbOtaOnError(ota_error_t error);
void cbOtaOnProgress(unsigned int progress, unsigned int total);

#endif