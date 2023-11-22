/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00009800-0000-1000-8000-00177a000002");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("0000aa00-0000-1000-8000-00177a000002");


// responses
const std::vector<uint8_t> aid = { 0xC0, 0x6F, 0x08, 0x85, 0x06, 0x02, 0x01, 0x40, 0x02, 0x01, 0x00, 0x90, 0x00 };
const std::vector<uint8_t> wrong = { 0xC0, 0x6A, 0x82 };
const std::vector<uint8_t> ack = { 0xC0, 0x90, 0x00 };
const std::vector<uint8_t> part1 = { 0x81, 0x44, 0x3E, 0x44, 0x00, 0x00, 0x00, 0xA6, 0x13, 0xA1, 0x11, 0xA1, 0x0F, 0x80, 0x01, 0x50, 0x81, 0x01, 0x01, 0x82 };
const std::vector<uint8_t> part2 = { 0x40, 0x01, 0x01, 0x83, 0x01, 0x00, 0x84, 0x01, 0x01, 0x90, 0x00 };
const std::vector<uint8_t> beep = { 0xC0, 0x44, 0x3E, 0x44, 0x00, 0x00, 0x00, 0xA6, 0x06, 0xA0, 0x04, 0x85, 0x02, 0x1F, 0x40, 0x90, 0x00 };
const std::vector<uint8_t> longbeep = { 0xC0, 0x44, 0x3E, 0x44, 0x00, 0x00, 0x00, 0xA6, 0x06, 0xA0, 0x04, 0x85, 0x02, 0x7D, 0x00, 0x90, 0x00 };




//global variables malas
static BLEAdvertisedDevice* myDevice;
int count = 0;                                                    // Add this line to declare the count variable
BLEClient* pClient;                                               //lo siento
BLEScan* pBLEScan;                                                //pero esto no lo va a ver nadie, verdad?
static BLERemoteService* pRemoteService = nullptr;                // Initialize the remote service pointer
static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;  // Initialize the remote characteristic pointer
//las no tqan malas pero que se me acumulan
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static boolean conectado = false;

//18/11
//La idea de esto es sacar el write fuera del notify para que este sea lo mas rápido posible,
//quiero tener un string global para recibir los hex y un flag para saber si se ha respondido a eso ya

static boolean tratada = true;
static String recibido = "";



const unsigned long WRITE_TIMEOUT = 5000;  // posiblemente una idea mala pero son las 2am so u know...




String hexBytesToString(const std::vector<uint8_t>& bytes) {
  String result;
  for (const auto& byte : bytes) {
    char buffer[3];
    sprintf(buffer, "%02X", byte);
    result += buffer;
  }
  return result;
}

// Imprime lo que queda de memoria... porque alguien se está comiendo mi puto heap mem
void printFreeHeap(const char* location) {
  Serial.print("Free Heap @ ");
  Serial.print(location);
  Serial.print(": ");
  Serial.println(ESP.getFreeHeap());
}



/*
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {


    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      Serial.print("HID reader found: ");
      Serial.println(advertisedDevice.getAddress().toString().c_str());

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = false;

    }  // Found our server
  }    // onResult
};     // MyAdvertisedDeviceCallbacks


static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {




  // Convert the received data to a vector for comparison
  std::vector<uint8_t> receivedData(pData, pData + length);

  //convertimos el vector en string porque ya no se me ocurre otra forma de comparar

  //la mas elegante sería
  std::string recbElegan((char*)pData, length);
  //ah pero la que funciona es esta
  String recbFeo = hexBytesToString(receivedData);




  Serial.print("String feo pero formal: ");
  Serial.println(recbFeo);
  recibido = recbFeo;
  tratada = false;
}



/*

  if (receivedData == std::vector<uint8_t>{ 0xC0, 0x0A, 0x44, 0x0A, 0x00, 0x00, 0x00, 0xA6, 0x13, 0xA1, 0x11, 0xA1, 0x0F, 0x80, 0x01, 0x50, 0x81, 0x01, 0x01, 0x82 }) {
    Serial.println("AID sent");
    pBLERemoteCharacteristic->writeValue(const_cast<uint8_t*>(aid.data()), aid.size(), true);
  } else if (receivedString == "C000A404000AA000000440000101000100") {
    Serial.print("conectado?:"); Serial.println(conectado);
    pBLERemoteCharacteristic->writeValue(const_cast<uint8_t*>(wrong.data()), wrong.size(), true);
    Serial.println("WRONG sent");    
  } else if (receivedData == std::vector<uint8_t>{ 0xC0, 0x0D, 0x73, 0x00 }) {
    Serial.println("ACK2 sent");
    pBLERemoteCharacteristic->writeValue(const_cast<uint8_t*>(ack.data()), ack.size(), true);
  } else if (receivedData.size() >= 21 && receivedData[0] == 0xC0 && receivedData[15] == 0x50 && receivedData[16] == 0x81 && receivedData[17] == 0x01 && receivedData[18] == 0x01 && receivedData[19] == 0x82) {
    Serial.println("PART1 sent");
    pBLERemoteCharacteristic->writeValue(const_cast<uint8_t*>(part1.data()), part1.size(), true);
  } else if (receivedData.size() == 11 && receivedData[0] == 0x40 && receivedData[3] == 0x83 && receivedData[6] == 0x84 && receivedData[9] == 0x90 && receivedData[10] == 0x00) {
    Serial.println("PART2 sent");
    pBLERemoteCharacteristic->writeValue(const_cast<uint8_t*>(part2.data()), part2.size(), true);
  } else if (receivedData == std::vector<uint8_t>{ 0xC0, 0x90, 0x00 }) {
    Serial.println("ACK sent");
    pBLERemoteCharacteristic->writeValue(const_cast<uint8_t*>(ack.data()), ack.size(), true);
  } else if (receivedData == std::vector<uint8_t>{ 0xC0, 0x44, 0x3E, 0x44, 0x00, 0x00, 0x00, 0xA6, 0x06, 0xA0, 0x04, 0x85, 0x02, 0x1F, 0x40, 0x90, 0x00 }) {
    Serial.println("BEEP sent");
    pBLERemoteCharacteristic->writeValue(const_cast<uint8_t*>(beep.data()), beep.size(), true);
  } else if (receivedData == std::vector<uint8_t>{ 0xC0, 0x44, 0x3E, 0x44, 0x00, 0x00, 0x00, 0xA6, 0x06, 0xA0, 0x04, 0x85, 0x02, 0x7D, 0x00, 0x90, 0x00 }) {
    Serial.println("LONG BEEP");
    pBLERemoteCharacteristic->writeValue(const_cast<uint8_t*>(longbeep.data()), longbeep.size(), true);
  } else {
    Serial.println("Unhandled response");
  }
  */





class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    //Serial.print(pBLERemoteCharacteristic->readRawData().toString().c_str());
    conectado = 1;
    Serial.println("Estamos conectados!");
  }

  void onDisconnect(BLEClient* pclient) {
    conectado = 0;
    Serial.print("Desconectados...");
  }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());



  BLEClient* pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remote BLE Server.
  if (pClient->connect(myDevice)) {
    Serial.println(" - Connected to server");

    // Obtain a reference to the service in the remote BLE server.
     pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService != nullptr) {
      Serial.println(" - Found our service");

      // Obtain a reference to the characteristic in the service.
      pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
      if (pRemoteCharacteristic != nullptr) {
        Serial.println(" - Found our characteristic");

        // Register for notifications
        if (pRemoteCharacteristic->canNotify()) {

          // Attempt to register for notifications
          pRemoteCharacteristic->registerForNotify(notifyCallback);

          Serial.println(" - Registered for notifications");


          connected = true;
          return true;
        } else {
          Serial.println("Characteristic does not support notifications");
        }
      } else {
        Serial.println("Failed to find our characteristic UUID");
      }
    } else {
      Serial.println("Failed to find our service UUID");
    }
  } else {
    Serial.println("Failed to connect to the server");
  }

  // Disconnect on failure
  pClient->disconnect();
  connected = false;
  Serial.println("Cliente desconectado");


  return false;
}


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");



  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}  // End of setup.


void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  try {
    if (doConnect == true) {
      Serial.println("Intentando conectar al lector");
      if (connectToServer()) {
        Serial.println("Conectados al servidor BLE");
      } else {
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      }
      doConnect = false;
    }
  } catch (String e) {
    Serial.println("Error intentando conectar"
                   "+e+");
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {

    Serial.println("Esperando a al puto...");
    if (!tratada) {
      Serial.println("Tratando notify...");
      if ((recibido=="")&& (recibido == "C000A404000AA000000440000101000100")) {
        pRemoteCharacteristic->writeValue(const_cast<uint8_t*>(wrong.data()), wrong.size(), true);
        Serial.println("WRONG sent");
        tratada=true;
      }
    }


    } else if (doScan) {
      BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
    }

    delay(1000);  // Delay a second between loops.
  }

