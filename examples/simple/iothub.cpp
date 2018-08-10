#include <WiFiClientSecure.h>
#include <AzureIoTHubMQTTClient.h>

#define IOTHUB_HOSTNAME         "<your-hub-name-here>.azure-devices.net"
#define DEVICE_ID               "<id of the device>"
#define DEVICE_KEY              "<device secret key>"


WiFiClientSecure tlsClient;
AzureIoTHubMQTTClient client(tlsClient, IOTHUB_HOSTNAME, DEVICE_ID, DEVICE_KEY);

void onMessageCallback(const MQTT::Publish& msg);
void onClientEvent(const AzureIoTHubMQTTClient::AzureIoTHubMQTTClientEvent event);

void initHubConnection()
{
    client.onEvent(onClientEvent);

    Serial.print("\nBeginning Azure IoT Hub Client... ");
    if (client.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("Could not connect to MQTT");
    }
}

void runHubConnection()
{
    client.run();
}

void onClientEvent(const AzureIoTHubMQTTClient::AzureIoTHubMQTTClientEvent event) {
    if (event == AzureIoTHubMQTTClient::AzureIoTHubMQTTClientEventConnected) {
        Serial.println("Connected to Azure IoT Hub");
        client.onMessage(onMessageCallback);
    }
}

void onMessageCallback(const MQTT::Publish& msg) {
    Serial.println("onMessageCallback");
    if (msg.payload_len() == 0) {
        return;
    }

    Serial.println("Received message from IoT Hub!");
    Serial.println(msg.payload_string());

    // do your message processing here
    // parse json or whatever
    // I was doing plain text messages
    /*
    if (msg.payload_string() == "BOOKED")
        setLED(LED_YELLOW);
    else if (msg.payload_string() == "FREE")
        setLED(LED_GREEN);
    else if (msg.payload_string() == "BUSY")
        setLED(LED_RED);
    else
        setLED(LED_BLUE);
    */
}

void sendHubEvent(const char *msg) {
    client.sendEvent(String("{ \"eventType\": \"") + msg + "\" }");
}