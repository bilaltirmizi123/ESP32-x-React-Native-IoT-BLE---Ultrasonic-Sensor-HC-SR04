/*
  Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  updated by chegewara and MoThunderz
*/


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Initialize all pointers
BLEServer* pServer = NULL;                    // Pointer to the server
BLECharacteristic* pCharacteristic_1 = NULL;  // Pointer to Characteristic 1

BLE2902* pBLE2902_1;  // Pointer to BLE2902 of Characteristic 1


// Some variables to keep track on device connected
bool deviceConnected = false;
bool oldDeviceConnected = false;

const int TRIG_PIN = 5;   // Trig pin of the HC-SR04 sensor
const int ECHO_PIN = 18;  // Echo pin of the HC-SR04 sensor
const int SPEED_OF_SOUND = 0.343;
const int LED_PIN = 23;

int distance = 0;
String data = "";


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
// UUIDs used in this example:
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_1 "beb5483e-36e1-4688-b7f5-ea07361b26a8"  // CHARACTERISTIC UUID FOR RED LED



// Callback function that is called whenever a client is connected or disconnected
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);


  // Create the BLE Device
  BLEDevice::init("ESP32 Microcontroller");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic_1 = pService->createCharacteristic(
    CHARACTERISTIC_UUID_1,
    BLECharacteristic::PROPERTY_NOTIFY);


  // Create a BLE Descriptor
  pBLE2902_1 = new BLE2902();
  pBLE2902_1->setNotifications(true);
  pCharacteristic_1->addDescriptor(pBLE2902_1);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}


void loop() {

  if (deviceConnected) {

    // Clear the trigPin by setting it LOW for 2 microseconds
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    // Set the trigPin HIGH for 10 microseconds
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Read the echoPin, and calculate the duration in microseconds
    long duration = pulseIn(ECHO_PIN, HIGH);

    // Calculate the distance in centimeters
    // Speed of sound is approximately 343 meters per second
    // The formula is distance = (duration * speed of sound) / 2
    distance = duration * 0.0343 / 2;
    data = String(distance);
    pCharacteristic_1->setValue(data.c_str());
    pCharacteristic_1->notify();

    if(distance < 20){
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
    } else {
      digitalWrite(LED_PIN, LOW);
    }

    // Print the distance to the Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // In this example "delay" is used to delay with one second. This is of course a very basic
    // implementation to keep things simple. I recommend to use millis() for any production code
    delay(1000);
  }
  // The code below keeps the connection status uptodate:
  // Disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // Connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}