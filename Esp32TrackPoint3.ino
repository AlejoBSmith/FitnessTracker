#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 22
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];


#include <esp_now.h>
#include <WiFi.h>

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
  int e=3;     // Indica el punto 3 del recorrido
  int f;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

#define datos 6
int envio[datos];

//myData.e=4;

void setup() { 
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
    
  }

  // Once ESPNow is successfully Init, we will register for Send CB to get the status of Trasnmitted packet
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
 
void loop() {

  static unsigned long lastWireTransmit = 0;
      if (millis() - lastWireTransmit > 5000) {    // Verifica que el TAGs no se acerque hasta después de 5 segs de ya haberse leido.
        nuidPICC[0]=0;
        nuidPICC[1]=0;
        nuidPICC[2]=0;
        nuidPICC[3]=0;
        lastWireTransmit = millis();
        }
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3] ) {
    
    // Serial.println(F("A new card has been detected."));  
  
    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
    myData.a = rfid.uid.uidByte[0];
    myData.b = rfid.uid.uidByte[1];
    myData.c = rfid.uid.uidByte[2];
    myData.d = rfid.uid.uidByte[3];
    //myData.e = 3;
    myData.f = 1;
   
    //if (myData.e==4){myData.e=3;}else{if(myData.e==3){myData.e=4;}} //Comentar esta línea para la instalación final.

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
     
    if (result == ESP_OK) {
      Serial.println("Sent with success");
      Serial.print(myData.a);
      Serial.print(" ");
      Serial.print(myData.b);
      Serial.print(" ");
      Serial.print(myData.c);
      Serial.print(" ");
      Serial.print(myData.d);
      Serial.print(" ");
      Serial.print(myData.e);
      Serial.print(" ");
      Serial.println(myData.f);
    }
    else {
      Serial.println("Error sending the data");
    }

    
  }else {
    //Serial.println(F("Card read previously."));
    lastWireTransmit = millis();
  }
      
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  delay(300);

}
  
