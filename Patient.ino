#define BLYNK_TEMPLATE_ID "TMPL3zaQDoknE"
#define BLYNK_TEMPLATE_NAME "Health Monitoring System"
#define BLYNK_AUTH_TOKEN "ml07tZI-NEQxEc3LEba9_MZ0BMj9v0x7"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <PulseSensorPlayground.h> // Include the pulse sensor library
#include <WiFiClientSecure.h>
#include <base64.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "123";
char pass[] = "12345678";

const int LM35Pin = 0; // Analog pin for LM35 temperature sensor
const int PulseSensorPin = A0; // Analog pin for pulse sensor

PulseSensorPlayground pulseSensor; // Create a pulse sensor object

float tempThreshold = 37.0; // Initial temperature threshold in Celsius
int pulseThreshold = 100; // Initial pulse rate threshold

// Twilio account credentials
const char* twilioAccountSID = "AC9d640dd3f04e59956666d61c7b871e5a";
const char* twilioAuthToken = "c6a6aa0d3cbab9bcc784fb37385e2a45";
const char* twilioPhoneNumber = "+17609708339";
const char* doctorPhoneNumber = "+919488230126";
const char* patientPhoneNumber = "+919566188859";

WiFiClientSecure client;

void setup() {
  Serial.begin(9600);
  
  pinMode(LM35Pin, INPUT);
  pinMode(PulseSensorPin, INPUT);

  Blynk.begin(auth, ssid, pass);

  // Initialize the pulse sensor
  pulseSensor.begin();       
  pulseSensor.setThreshold(550);
  pulseSensor.analogInput(PulseSensorPin);
}

void loop() {
  Blynk.run();
  
  // Read temperature
  float temperature = getTemperature();
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Read pulse
  int pulse = pulseSensor.getBeatsPerMinute();

  // Print pulse
  Serial.print("Pulse: ");
  Serial.print(pulse);
  Serial.println(" BPM");
  
  // Send temperature and pulse data to Blynk
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, pulse);

  // Check if pulse crosses threshold and send alert
  if (pulse < 60 || pulse > 100) {
    sendTwilioSMS("Abnormal + rate detected! Please meet patient Gowtham @+919566188859");
    sendTwilioSMS1("Please Meet Doctor! Your Pulse is abnormal");
  }

  delay(1000);
}

float getTemperature() {
  int raw_myTemp = analogRead(LM35Pin);
  float tempC = (raw_myTemp * 3.3 / 1024.0 * 10); // LM35 provides 10mV per degree Celsius
  return tempC;
}

void sendTwilioSMS(const char* message) {
  String encodedCredentials = base64::encode(String(twilioAccountSID) + ":" + String(twilioAuthToken));
  client.setInsecure();
  
  if (client.connect("api.twilio.com", 443)) {
    String postData = "To=" + String(doctorPhoneNumber) + "&From=" + String(twilioPhoneNumber) + "&Body=" + String(message);
    client.println("POST /2010-04-01/Accounts/" + String(twilioAccountSID) + "/Messages.json HTTP/1.1");
    client.println("Host: api.twilio.com");
    client.println("Authorization: Basic " + encodedCredentials);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(postData.length()));
    client.println("Connection: close");
    client.println();
    client.println(postData);
  } else {
    Serial.println("Error sending message to Twilio.");
  }
}

void sendTwilioSMS1(const char* message) {
  String encodedCredentials = base64::encode(String(twilioAccountSID) + ":" + String(twilioAuthToken));
  client.setInsecure();
  
  if (client.connect("api.twilio.com", 443)) {
    String postData = "To=" + String(patientPhoneNumber) + "&From=" + String(twilioPhoneNumber) + "&Body=" + String(message);
    client.println("POST /2010-04-01/Accounts/" + String(twilioAccountSID) + "/Messages.json HTTP/1.1");
    client.println("Host: api.twilio.com");
    client.println("Authorization: Basic " + encodedCredentials);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(postData.length()));
    client.println("Connection: close");
    client.println();
    client.println(postData);
  } else {
    Serial.println("Error sending message to Twilio.");
  }
}
