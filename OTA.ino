/*
My better version of Arduino OTA example. Includes simple show value via AJAX

Ajax related code from
https://circuits4you.com/2018/11/20/web-server-on-esp32-how-to-update-and-display-sensor-values/

OTA example from Arduino-ESP

kolin 2019
*/


#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include "webpage.h"

const char *ssid = "SSID";
const char *password = "password";

WebServer server(80);


void setup(void)
{
    Serial.begin(115200);

    // Connect to WiFi network
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, handleIndexPage);
    server.on("/firmware", HTTP_GET, handleFirmwarePage);
    server.on("/firmwareupdate", HTTP_POST, handleFirmware1, handleFirmware2);
    server.on("/readADC", handleADC); //To get update of ADC Value only
    server.begin();
}

void loop(void)
{
    server.handleClient();
    delay(1);
}

void handleADC()
{
    int a = random(1024);
    String adcValue = String(a);
    server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
}

void handleIndexPage()
{
    String s = INDEX_page;
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", s);
}

void handleFirmwarePage()
{
    String s = FIRMWARE_page;
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", s);
}

void handleFirmware1()
{
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
}

void handleFirmware2()
{
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        { //start with max available size
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        { //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        }
        else
        {
            Update.printError(Serial);
        }
    }
}