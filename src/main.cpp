#include <main.h>

char _hostname[27];

bool blinkState = false;
volatile uint8_t seconds = 0;
WiFiEventHandler onStationModeGotIPhandler, onStationModeDisconnectedHandler;

// ESP8266WebServer m_WebServer(80);

RtcDS3231<TwoWire> m_Rtc(Wire);

WiFiUDP m_NtpUdp;
Scheduler m_Scheduler;

SoftwareSerial m_Display(-1, D8);

Task m_TaskNtpUpdate(100, 25, &cbNtpUpdate, &m_Scheduler, false, &cbNtpUpdateEnable, &cbNtpUpdateDisable);

// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};   // Central European Standard Time
Timezone m_Timezone(CEST, CET);

void handleRoot()
{
  // m_WebServer.send(200, "text/plain", "hello from esp8266!");
}

void handleWebUpdate()
{
  // m_WebServer.send(200, "text/plain", "triggering NTP update");

  if (!m_TaskNtpUpdate.isEnabled())
  {
    m_TaskNtpUpdate.restartDelayed(TASK_SECOND);
  }
}

void onStationConnected(const WiFiEventStationModeGotIP &evt)
{
  schedule_function([](void) {
    Serial.print(F("[WiFi] Connected: "));
    Serial.println(WiFi.localIP());

    if (MDNS.begin(_hostname))
    {
      Serial.println(F("[MDNS] Responder started"));
    }

    if (!m_TaskNtpUpdate.isEnabled())
    {
      m_TaskNtpUpdate.restartDelayed(TASK_SECOND);
    }
  });
}

void onStationDisconnected(const WiFiEventStationModeDisconnected &evt)
{
  schedule_function([](void) {
    Serial.println(F("[WiFi] Disconnected"));
  });
}

void ICACHE_RAM_ATTR handleRtcInterrupt()
{
  seconds++;
}

void setup()
{
  sprintf(_hostname, "esp8266-%06x-clock.local", ESP.getChipId());

  Serial.begin(74880, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.println();
  Serial.print(F("Hostname: "));
  Serial.println(_hostname);
  //Serial.println(ESP.getFullVersion());

  m_Display.begin(9600, SWSERIAL_8N1);
  m_Display.print('v');

  m_Display.write(0x7A);    // Brightness control command
  m_Display.write((byte)0xff); // dimmest value (must type-def 0)

  m_Display.write(0x79);
  m_Display.write(0x00);
  m_Display.write("BOOT");

  onStationModeGotIPhandler = WiFi.onStationModeGotIP(&onStationConnected);
  onStationModeDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationDisconnected);

  // m_WebServer.on("/", handleRoot);
  // m_WebServer.on("/update", handleWebUpdate);

  // rtc 1Hz interrupt
  pinMode(D7, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(D7), handleRtcInterrupt, FALLING);

  m_Rtc.Begin();                                                   //Starts I2C
  m_Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock);           //Sets pin mode
  m_Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz); //Sets frequency

  if (!m_Rtc.GetIsRunning())
  {
    Serial.println("[RTC] Not actively running, starting now");
    m_Rtc.SetIsRunning(true);
  }

  WiFi.mode(WIFI_STA);
  WiFi.hostname(_hostname);
  WiFi.begin("iot");

  ArduinoOTA.onStart(cbOtaOnStart);
  ArduinoOTA.onEnd(cbOtaOnEnd);
  ArduinoOTA.onError(cbOtaOnError);
  ArduinoOTA.onProgress(cbOtaOnProgress);
  ArduinoOTA.setHostname(_hostname);

  m_NtpUdp.begin(2390);
  // m_WebServer.begin();
  ArduinoOTA.begin(false);

  delay(1000);
}

void loop()
{
  m_Scheduler.execute();

  if (seconds > 0)
  {
    seconds--;
    blinkState = !blinkState;

    byte data = blinkState ? 0x10 : 0x00;
    if (m_TaskNtpUpdate.isEnabled())
      data |= 0x01;
    if (WiFi.status() != WL_CONNECTED)
      data |= 0x02;

    m_Display.write(0x77); // Decimal control command
    m_Display.write(data);

    RtcDateTime dt = m_Rtc.GetDateTime();

    time_t now = m_Timezone.toLocal(dt.Epoch32Time());

    m_Display.write(0x79);
    m_Display.write(0x00);
    m_Display.printf("%02u%02u", hour(now), minute(now));

    Serial.printf("%02u.%02u.%04u %02u:%02u:%02u\r\n", day(now), month(now), year(now), hour(now), minute(now), second(now));
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    MDNS.update();
    ArduinoOTA.handle();

    // m_WebServer.handleClient();
  }
}
