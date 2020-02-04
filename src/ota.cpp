#include <main.h>

void cbOtaOnStart()
{
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
        type = "sketch";
    }
    else
    { // U_FS
        type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("[OTA] Start updating " + type);
}

void cbOtaOnEnd()
{
    Serial.println();
    Serial.println("[OTA] End");
}

void cbOtaOnProgress(unsigned int progress, unsigned int total)
{
    Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
}

void cbOtaOnError(ota_error_t error)
{
    Serial.printf("[OTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
        Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
        Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
        Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
        Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
        Serial.println("End Failed");
    }
}