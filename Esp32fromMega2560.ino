// WireSlave Receiver
// by Gutierrez PS <https://github.com/gutierrezps>
// ESP32 I2C slave library: <https://github.com/gutierrezps/ESP32_I2C_Slave>
// based on the example by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the WireSlave library for ESP32.
// Receives data as an I2C/TWI slave device; data must
// be packed using WirePacker.
// Refer to the "master_writer" example for use with this

#include <Arduino.h>
#include <Wire.h>
#include <WireSlave.h>

#include <esp_now.h>
#include <WiFi.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_SLAVE_ADDR 0x04

#define datos 6
int envio[datos];

void receiveEvent(int howMany);

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x94, 0xB9, 0x7E, 0xE5, 0x11, 0xB0};    // Dirección del esp32 con capacitor soldado
//94:B9:7E:E5:AE:C8   esp32 con cap<
//94:B9

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int a;
  int b;
  int c;
  int d;
  int e;
  int f;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}




void setup()
{
  Serial.begin(115200);

  bool success = WireSlave.begin(SDA_PIN, SCL_PIN, I2C_SLAVE_ADDR);
  if (!success) {
     Serial.println("I2C slave init failed");
     while(1) delay(100);
  }

  WireSlave.onReceive(receiveEvent);

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;    
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
    
}

void loop()
{
    // the slave response time is directly related to how often
    // this update() method is called, so avoid using long delays
    // inside loop(), and be careful with time-consuming tasks
    WireSlave.update();

    // let I2C and other ESP32 peripherals interrupts work
    delay(1);

static unsigned long lastWireTransmit = 0;
if (millis() - lastWireTransmit > 1000) {
  //strcpy(myData.a, "Ana ensucio mi zapatilla :/");
//  myData.a = envio[0];
//  myData.b = envio[1];
//  myData.c = envio[2];
//  myData.d = envio[3];
//  myData.e = envio[4];
//  myData.f = envio[5];
//  // Send message via ESP-NOW
//  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
//   
//  if (result == ESP_OK) {
//    Serial.println("Sent with success");
//  }
//  else {
//    Serial.println("Error sending the data");
//  }
  lastWireTransmit = millis();
}
    
}

// function that executes whenever a complete and valid packet
// is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
    int cnt=0;  
    while (0 < WireSlave.available()) // loop through all but the last byte
    {
        int c = WireSlave.read();  // receive byte as a character
        envio[cnt]=c;
        cnt++;
       
    }
    for(int i =0; i<datos; i++){
      Serial.print(envio[i]);
      Serial.print(" ");
    }
    Serial.println();
    Serial.println(sizeof(myData));

    myData.a = envio[0];
    myData.b = envio[1];
    myData.c = envio[2];
    myData.d = envio[3];
    myData.e = envio[4];
    myData.f = envio[5];
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
     
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }










    

}
