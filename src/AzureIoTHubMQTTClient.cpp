//
// Created by Andri Yadi on 10/29/16.
//

#include "AzureIoTHubMQTTClient.h"

#include "sha256.h"
#include "Base64.h"
#include "Utils.h"
#include <ESP8266WiFi.h>
#include <Time.h>

AzureIoTHubMQTTClient::AzureIoTHubMQTTClient(Client& c, String iotHubHostName, String deviceId, String deviceKey):
        iotHubHostName_(iotHubHostName), deviceId_(deviceId), deviceKey_(deviceKey), PubSubClient(c, iotHubHostName, AZURE_IOTHUB_MQTT_PORT) {

    mqttCommandSubscribeTopic_ = "devices/" + deviceId + "/messages/devicebound/#";
    mqttCommandPublishTopic_ = "devices/" + deviceId + "/messages/events/";

    using namespace std::placeholders;
}

AzureIoTHubMQTTClient::~AzureIoTHubMQTTClient() {

}

bool AzureIoTHubMQTTClient::begin() {

    if (!WiFi.isConnected())	{
        DEBUGLOG("NOT connected to internet!\n");
        return false;
    }

    canConnect_ = true;

    return true;
}

void AzureIoTHubMQTTClient::run() {

    if (canConnect_) {
        canConnect_ = false;

        if (!connected()) {
            DEBUGLOG("calling doConnect...\n");
            doConnect();
        }
    }

    PubSubClient::loop();
}

void AzureIoTHubMQTTClient::end() {
    disconnect();
}

String AzureIoTHubMQTTClient::createIotHubSASToken(char *key, String url, long expire){

    url.toLowerCase();

    String stringToSign = url + "\n" + String(expire);

    // START: Create signature
    // https://raw.githubusercontent.com/adamvr/arduino-base64/master/examples/base64/base64.ino

    int keyLength = strlen(key);

    int decodedKeyLength = base64_dec_len(key, keyLength);
    char decodedKey[decodedKeyLength];  //allocate char array big enough for the base64 decoded key

    base64_decode(decodedKey, key, keyLength);  //decode key

    Sha256.initHmac((const uint8_t*)decodedKey, decodedKeyLength);
    Sha256.print(stringToSign);
    char* sign = (char*) Sha256.resultHmac();
    // END: Create signature

    // START: Get base64 of signature
    int encodedSignLen = base64_enc_len(HASH_LENGTH);
    char encodedSign[encodedSignLen];
    base64_encode(encodedSign, sign, HASH_LENGTH);

    // SharedAccessSignature
    return "sr=" + url + "&sig="+ urlEncode(encodedSign) + "&se=" + String(expire);
    // END: create SAS
}

bool AzureIoTHubMQTTClient::doConnect() {

    if (sasToken_.equals("")) {
        DEBUGLOG("Creating SAS Token!\n");

        String url = iotHubHostName_ + urlEncode(String("/devices/" + deviceId_).c_str());
        char *devKey = (char *)deviceKey_.c_str();
        long expire = time(NULL) + AZURE_IOTHUB_TOKEN_EXPIRE;
        DEBUGLOG("SAS Token expire: %d\n", expire);

        //TODO: Store SAS token? So that no expensive operation for each begin
        sasToken_ = createIotHubSASToken(devKey, url, expire);
    }

    changeEventTo(AzureIoTHubMQTTClientEventConnecting);

    String mqttUname =  iotHubHostName_ + "/" + deviceId_ + "/api-version=2016-11-14";
    String mqttPassword = "SharedAccessSignature " + sasToken_;
    //DEBUGLOG(mqttPassword);

    MQTT::Connect conn = MQTT::Connect(deviceId_).set_auth(mqttUname, mqttPassword);//.set_clean_session();
    conn.set_keepalive(10);
    bool ret = PubSubClient::connect(conn);

    if (ret) {

        changeEventTo(AzureIoTHubMQTTClientEventConnected);

        PubSubClient::callback_t cb = [=](const MQTT::Publish& p){
            _onActualMqttMessageCallback(p);
        };

        set_callback(cb);

        // Subscribe
        bool subscribed = PubSubClient::subscribe(mqttCommandSubscribeTopic_, 0);

        if (!subscribed) {
            DEBUGLOG("Failed to subscribe!");
            return false;
        }
        
        return true;

    } else {

        DEBUGLOG("Failed to connect to Azure IoT Hub\n");
        return false;
    }
}

bool AzureIoTHubMQTTClient::sendEvent(String payload) {
    //return PubSubClient::publish(mqttCommandPublishTopic_, payload);
    return PubSubClient::publish(MQTT::Publish(mqttCommandPublishTopic_, payload).set_qos(1).set_retain(false));
}

bool AzureIoTHubMQTTClient::sendEvent(const uint8_t *payload, uint32_t plength, bool retained) {
    return PubSubClient::publish(mqttCommandPublishTopic_, payload, plength, retained);
}

void AzureIoTHubMQTTClient::_onActualMqttMessageCallback(const MQTT::Publish &msg) {
    DEBUGLOG("Message arrived!");
    //Last resort
    if (onSubscribeCallback_) {
        onSubscribeCallback_(msg);
    }
}

void AzureIoTHubMQTTClient::changeEventTo(AzureIoTHubMQTTClient::AzureIoTHubMQTTClientEvent event) {

    currentEvent_ = event;

    if (eventCallback_) {
        eventCallback_(event);
    }
}
