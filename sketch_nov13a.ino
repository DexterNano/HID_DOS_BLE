#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>

// Define your 128-bit UUIDs
BLEUUID serviceUUID("00009800-0000-1000-8000-00177a000002F");
BLEUUID characteristicUUID("0000aa00-0000-1000-8000-00177a000002");

// Callback function for characteristic updates
void onCharacteristicNotify(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  Serial.print("Notify update: ");
  for (int i = 0; i < length; i++) {
    Serial.print(pData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  // Initialize BLE
  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  BLEClient* pClient = nullptr;

  // Scan for devices
  BLEScanResults scanResults = pBLEScan->start(5);

  if (scanResults.getCount() == 0) {
    Serial.println("No devices found during scanning");
    return;
  } else {Serial.println("Device found");}

  for (int i = 0; i < scanResults.getCount(); i++) {
    if (scanResults.getDevice(i).haveServiceUUID() && scanResults.getDevice(i).isAdvertisingService(serviceUUID)) {
      BLEAdvertisedDevice device = scanResults.getDevice(i);
      Serial.println("Conectando al lector: "" + device.getAddress().toString().c_str() + ""  HID");
      // Connect to the device
      BLEAddress foundAddress = device.getAddress();
      pClient = BLEDevice::createClient();
      if (!pClient->connect(foundAddress)) {
        Serial.println("Failed to connect to the device");
        return;
      } else {Serial.println("Conectado");}

      // Get the service and characteristic
      BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
      if (pRemoteService == nullptr) {
        Serial.println("Failed to find the remote service");
        return;
      }

      BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(characteristicUUID);
      if (pRemoteCharacteristic == nullptr) {
        Serial.println("Failed to find the remote characteristic");
        return;
      } else {Serial.println("Caracteristica encontrada");}

      // Set notify callback
      pRemoteCharacteristic->registerForNotify(onCharacteristicNotify);

      Serial.println("Connected and notifications set up successfully!");
      break;
    }
  }
}

void loop() {
  // Your main loop code here
  // You can add any additional functionality or conditions as needed
}
