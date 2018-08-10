//
// Created by Andri Yadi on 10/29/16.
//

#ifndef PIOMQTTAZUREIOTHUB_AZUREIOTHUBMQTT_H
#define PIOMQTTAZUREIOTHUB_AZUREIOTHUBMQTT_H

#include <Arduino.h>
#include "PubSubClient.h"
#undef min
#undef max
#include <functional>
#include <vector>
#include <map>

#define DEBUG_PORT Serial
#define DEBUG

#ifdef DEBUG
#define DEBUGLOG(...) DEBUG_PORT.printf(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

#define AZURE_IOTHUB_MQTT_PORT    8883
#define AZURE_IOTHUB_TOKEN_EXPIRE   (10*24*3600) //seconds

class AzureIoTHubMQTTClient : public PubSubClient {
public:

    enum AzureIoTHubMQTTClientEvent {
        AzureIoTHubMQTTClientEventUnknown,
        AzureIoTHubMQTTClientEventConnecting,
        AzureIoTHubMQTTClientEventConnected,
        AzureIoTHubMQTTClientEventDisconnected
    };

    typedef std::function<void(const AzureIoTHubMQTTClientEvent event)> EventCallback;

    AzureIoTHubMQTTClient(Client& c, String iotHubHostName, String deviceId, String deviceKey);
    ~AzureIoTHubMQTTClient();

    bool begin();
    void run();
    void end();
    bool sendEvent(String payload);
    bool sendEvent(const uint8_t *payload, uint32_t plength, bool retained = false);
    bool setTimeZone(int timeZone);

    void onEvent(EventCallback cb) {
        eventCallback_ = cb;
    }

    AzureIoTHubMQTTClient& onMessage(callback_t cb) { onSubscribeCallback_ = cb; return *this; }

private:
    String iotHubHostName_;
    String deviceId_;
    String deviceKey_;
    String sasToken_;
    String mqttCommandSubscribeTopic_, mqttCommandPublishTopic_;
    PubSubClient::callback_t onSubscribeCallback_;
    volatile bool canConnect_ = false;

    EventCallback eventCallback_ = NULL;

    bool doConnect();
    String createIotHubSASToken(char *key, String url, long expire = 0);
    void _onActualMqttMessageCallback(const MQTT::Publish& p);

    AzureIoTHubMQTTClientEvent currentEvent_ = AzureIoTHubMQTTClientEventUnknown;
    void changeEventTo(AzureIoTHubMQTTClientEvent event);
};


#endif //PIOMQTTAZUREIOTHUB_AZUREIOTHUBMQTT_H
