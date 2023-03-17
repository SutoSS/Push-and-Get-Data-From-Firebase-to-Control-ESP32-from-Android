#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

//ID SpreadSheet
String GOOGLE_SCRIPT_ID = "******"; // Replace by your GAS service id

//updated 04.12.2019
const char * root_ca=\
"-----BEGIN CERTIFICATE-----\n" \
"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n" \
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n" \
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n" \
"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n" \
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n" \
"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n" \
"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n" \
"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n" \
"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n" \
"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n" \
"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n" \
"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n" \
"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n" \
"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n" \
"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n" \
"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n" \
"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n" \
"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n" \
"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n" \
"-----END CERTIFICATE-----\n";


#define ON_Board_LED 2

//milis time
unsigned long interval=10000;
unsigned long interval_1=3000;  
unsigned long previousMillis=0;

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

const int pir = 13;
const int sensor = 15;
int val =0;
int res_ml=0;
int gerak;

char tempString[8];

char serialData;

String sensorLocation = "Office";
String sensorName = "PIR";
String Status;
String Lampu;

int value_kontrol, ml_respon;

//Things to change
const char * ssid = "******";
const char * password =  "******";

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert Firebase project API Key
#define API_KEY "******" //karater API

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "******" //link firbase

//Email
#define USER_EMAIL "******";
#define USER_PASSWORD "******";

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

const char* mqtt_server = "192.168.1.110";

String apiKeyValue = "tPmAT5Ab3j7F9";
 
void setup()
{
  Serial.begin(115200);
  pinMode(sensor, INPUT);   
  pinMode(pir, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) 
  { 
    delay(500);
    Serial.print(".");
  }

  pinMode(ON_Board_LED,OUTPUT); 
  digitalWrite(ON_Board_LED, HIGH);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
  }

  digitalWrite(ON_Board_LED, HIGH);
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = "******";

  #if defined(ESP8266)
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
  #endif

  fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;//1000

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() 
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  movement();
}

void callback(char* topic, byte* message, unsigned int length) 
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  
  if (String(topic) == "smartlighting_pred/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
    }
    else if(messageTemp == "off"){
      Serial.println("off");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("smartlighting_pred/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void movement()
{
 val = digitalRead(sensor);
 gerak = val;
 ml();
 Status = "ml on";
 if(res_ml == 1)
 {
  On();
 }
 else
 {
  Off();
 }
 Serial.print("gerakan:");   
 Serial.println(gerak);
 senddata();
 bacaperintah();
 if(value_kontrol == 0)
 {
  delay(50);
  kontrol();
 }
}

void kontrol()
{
  case1:
        bacaperintah();
        val = digitalRead(sensor);
        if(value_kontrol == 2)
        {
          delay(100);
          goto case2;
        }
        else if(value_kontrol == 3)
        {
          delay(100);
          return loop();
        }
        gerak=val;
        senddata();
        Status="Manual On";
        Serial.println(Status);
        On();
        goto case1;
          
 case2:
       bacaperintah();
       val = digitalRead(sensor);
       if(value_kontrol == 1)
       {
         delay(100);
         goto case1;
       }
       else if(value_kontrol == 3)
       {
         delay(100);
         return loop();
       }
       gerak=val;
       senddata();
       Status="Manual Off";
       Off();
       Serial.println(Status);
       goto case2;
}


void bacaperintah()
{
 unsigned long currentMillis = millis();
 if ((unsigned long)(currentMillis - previousMillis) >= interval_1)
 {
    if (Firebase.RTDB.getInt(&fbdo, "/PIR/status")) 
    {
        if (fbdo.dataType() == "int") 
        {
          value_kontrol = fbdo.intData();
          Serial.println(value_kontrol);
        }
    }
    else 
    {
        Serial.println(fbdo.errorReason());
    }
    delay(50);
 }
}

void ml()
{
 if (Firebase.RTDB.getInt(&fbdo, "/PIR/ml")) 
 {
  if (fbdo.dataType() == "int") 
  {
   ml_respon = fbdo.intData();
   res_ml = ml_respon;
   Serial.println(ml_respon);
   Serial.println(Status);
  }
 }
 else 
 {
  Serial.println(fbdo.errorReason());
 }
}

void On()
{
   digitalWrite(pir, LOW);
   Lampu = "On";
   delay(100);
}

void Off()
{
   digitalWrite(pir, HIGH);
   Lampu = "Off";
   delay(100);
}

void senddata()
{
 unsigned long currentMillis = millis();
 if ((unsigned long)(currentMillis - previousMillis) >= interval) 
 {
  dtostrf(gerak, 1, 2, tempString);
  client.publish("smartlighting", tempString);
  sendData("&PIR="+String(gerak)+"&Ml="+String(res_ml));
 }
}

void sendData(String params) 
{
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
   Serial.print(url);
    Serial.print("Making a request");
    http.begin(url.c_str()); //Specify the URL and certificate
    int httpCode = http.GET();  
    http.end();
    Serial.println(": done "+httpCode);
}
 
