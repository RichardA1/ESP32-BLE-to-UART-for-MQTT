
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <FastLED.h>

#define NUM_LEDS 64
#define DATA_PIN 26
CRGB leds[NUM_LEDS];


#define BRIGHTNESS  255
#define FRAMES_PER_SECOND  60

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;
String MSSAGE;
String HEXCOLOR;
String Mtype;
bool POWER = true;
String MODE = "1";
byte R = 0;
byte G = 0;
byte B = 255;


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        MSSAGE = "";
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++){
          MSSAGE = String(MSSAGE + rxValue[i]);
        }
        Serial.println(MSSAGE);
        Serial.println("*********");
        if (String(rxValue[0]) == "!"){
          if (String(rxValue[1]) == "P"){ // Power ON/OFF
            if (String(rxValue[2]) == "1"){ 
              POWER = true;
            } else {
              POWER = false;
            }
            Serial.print("Power is ");
            Serial.println(POWER);
          }
          if (String(rxValue[1]) == "M"){ // Mode Solector
            MODE = String(rxValue[2]);
            Serial.print("Mode is ");
            Serial.println(MODE);
          }
          if (String(rxValue[1]) == "F"){ // Flash
            
          }
          if (String(rxValue[1]) == "W"){ // Whipe
            
          }
          //Mtype = 
        }
        if (String(rxValue[0]) == "#"){
          HEXCOLOR = "";
          for (int i = 1; i < rxValue.length(); i++){
            HEXCOLOR = String(HEXCOLOR + rxValue[i]);
          }
          Serial.print("Color is 0x");
          Serial.println(HEXCOLOR);

          char charbuf[8];
          HEXCOLOR.toCharArray(charbuf,8);
          long int rgb=strtol(charbuf,0,16); //=>rgb=0x001234FE;
          R=(byte)(rgb>>16);
          G=(byte)(rgb>>8);
          B=(byte)(rgb);

          Serial.print("RGB: ");
          Serial.print(R);
          Serial.print(", ");
          Serial.print(G);
          Serial.print(", ");
          Serial.println(B);
          
        }
      }
    }
};


void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("BLE Thing M5");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
}

int POS = 0;

void loop() {
  if (deviceConnected) {
    Serial.print("LED POS:");
    Serial.println(POWER);
    if (POWER){
      if (MODE == "1"){
        colorFill(CRGB(R,G,B));
      }
      if (MODE == "2"){
        if (POS == NUM_LEDS)POS = 0;
        leds[POS] = CRGB(R,G,B);        //  Set pixel's color (in RAM)
        POS++;
        Serial.print("LED POS:");
        Serial.println(POS);
        FadeALL();
        FastLED.show();                        //  Update strip to match
        delay(40);                           //  Pause for a moment
      }
      //colorWipe(CRGB(R,G,B), 40);
    } else {
      colorFill(CRGB(0,0,0));
    }
    //pTxCharacteristic->setValue(&txValue, 1);
    //pTxCharacteristic->notify();
    //txValue++;
    delay(10); // bluetooth stack will go into congestion, if too many packets are sent
  }
  if (MSSAGE == "!CON"){
    Serial.println("Switch ON");
    colorWipe(CRGB::Green, 20); // Green
    MSSAGE = "";
  }
  if (MSSAGE == "!COFF"){
    Serial.println("Switch OFF");
    colorWipe(CRGB::Red, 20); // Red
    MSSAGE = "";
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
  // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }  
}



void colorFill(CRGB color) {
  for(int i=0; i<NUM_LEDS; i++) { // For each pixel in strip...
    leds[i] = color;        //  Set pixel's color (in RAM)
  }
  FastLED.show();                        //  Update strip to match
}

void colorWipe(CRGB color, int wait) {
  for(int i=0; i<NUM_LEDS; i++) { // For each pixel in strip...
    leds[i] = color;        //  Set pixel's color (in RAM)
    FadeALL();
    FastLED.show();                        //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void FadeALL() {
  for(int j=0; j<NUM_LEDS; j++) { // For each pixel in strip...
    leds[j].fadeToBlackBy( 10 ); //  Set pixel's color (in RAM)
  }
}
