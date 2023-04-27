#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAddress.h>
//--- sleep mode
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 5        /* Time ESP32 will go to sleep (in seconds) */
//---- ibeacon
String Adresse = "6b:6d:fe:a6:7b:49";       // change your address ibeacon
//----- ble
BLEScan* pBLEScan;
static BLEAddress* pServerAddress;
unsigned int scanTime = 1;  //In seconds
signed int rssi;
bool stt;

hw_timer_t* timer = NULL;
RTC_DATA_ATTR volatile uint32_t timeReset = 0;
RTC_DATA_ATTR volatile uint32_t timeReboot = 10;

volatile uint32_t timeDelay = 0;
volatile int timeSetRelay = 15;

const int relay = 15;
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    pServerAddress = new BLEAddress(Adresse.c_str());
    Serial.printf(advertisedDevice.getAddress().toString().c_str());
    Serial.print("---- RSSI: ");
    int rssi = (int)advertisedDevice.getRSSI();
    Serial.println(rssi);
    if (advertisedDevice.getAddress().equals(*pServerAddress) && (rssi > -60)) {
      Serial.print("Finded Ibeacon !!!");
      digitalWrite(relay, 1);
      timeDelay = 0;
      Serial.print("Relay On !!");
      advertisedDevice.getScan()->stop();
    }  // Found our server
    Serial.println("");
  }
};
void IRAM_ATTR onTimer() {
  //timeReset++;
  timeDelay++;

  if (timeDelay >= timeSetRelay) {
    digitalWrite(relay, 0);  // switch off after delay
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  //------ TIMER DELAY HOLD STATE RELAY
  timer = timerBegin(0, 80, true);  //  TIMER 0 DIV 80
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);  // timer 1s
  timerAlarmEnable(timer);
  //----- INIT BLE SCAN
  Serial.println("");
  Serial.println("Start BLE Scanner");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
}

void loop() {
  pBLEScan->start(scanTime);
  delay(1000);
}
