#include <ESP8266WiFi.h>
#include <time.h>
#include <coredecls.h>

#include "iothub.h"

const char *wifiName = "ssid";
const char *wifiPassword = "password";

void onTimeRetrieved();

void setup()
{
    Serial.begin(9600);
    settimeofday_cb(onTimeRetrieved);
    configTime(0, 0, "pool.ntp.org");
    WiFi.begin(wifiName, wifiPassword);

    // other initialization here
}

void onTimeRetrieved()
{
  Serial.printf("Current timestamp: %d\n", time(NULL));
  initHubConnection();
}

void loop()
{
    // some other periodical stuff here, in my case, I was doing smth like this:
    //if (checkButtonPress())
    //    sendHubEvent("buttonPressed");
    runHubConnection();
    delay(20);
}
