#include <main.h>

#include <TaskScheduler.h>

const int NTP_PACKET_SIZE = 48;     // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

bool cbNtpUpdateEnable()
{
  Serial.println(F("[NTP] Update started"));

  IPAddress ip;

  if (WiFi.hostByName("pool.ntp.org", ip))
  {
    Serial.print(F("[NTP] Using ntp server pool.ntp.org, ip="));
    Serial.println(ip);

    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);

    // Initialize values needed to form NTP request
    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision

    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    m_NtpUdp.beginPacket(ip, 123); //NTP requests are to port 123
    m_NtpUdp.write(packetBuffer, NTP_PACKET_SIZE);
    m_NtpUdp.endPacket();

    return true;
  }
  else
  {
    return false;
  }
}

void cbNtpUpdateDisable()
{
  Serial.println(F("[NTP] Update finished"));
}

void cbNtpUpdate()
{
  int cb = m_NtpUdp.parsePacket();
  if (!cb)
  {
    Serial.println(F("[NTP] No packet yet"));
  }
  else
  {
    Serial.print(F("[NTP] Packet received, length="));
    Serial.println(cb);
    // We've received a packet, read the data from it
    m_NtpUdp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // Serial.print(F("[NTP] Seconds since Jan 1 1900 = "));
    // Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print(F("[NTP] Unix time = "));
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

    time_t now = epoch; // m_Timezone.toLocal(epoch);

    Serial.printf("[NTP] UTC time =  %02u.%02u.%04u %02u:%02u:%02u\r\n", day(now), month(now), year(now), hour(now), minute(now), second(now));

    RtcDateTime timeToSet;

    timeToSet.InitWithEpoch32Time(epoch);
    m_Rtc.SetDateTime(timeToSet);

    m_Scheduler.currentTask().disable();
  }
}