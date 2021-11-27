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

// Structure example to receive data
// Must match the sender structure
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

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print(myData.a);
  Serial.print("-"); 
  Serial.print(myData.b); 
  Serial.print("-");
  Serial.print(myData.c);
  Serial.print("-");
  Serial.print(myData.d);
  Serial.print(", ");
  Serial.print(myData.e);
  Serial.print(", "); 
  Serial.println(myData.f);
  Serial.println();
}
 
void setup() {
  // Initialize Serial Monitor
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
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {


 // Para el registro de nuevos TAGs
 
  if ( ! rfid.PICC_IsNewCardPresent())   
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  //if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3] ) {
    
    // Serial.println(F("A new card has been detected."));  
  
    // Store NUID into nuidPICC array
   // for (byte i = 0; i < 4; i++) {
     // nuidPICC[i] = rfid.uid.uidByte[i];
   // }
      //Serial.println("Sent with success");

      for(int i=0;i<10;i++){   //Imprime 10 veces para garantizar el envío
        
      
      Serial.print(rfid.uid.uidByte[0]);
      Serial.print("-"); 
      Serial.print(rfid.uid.uidByte[1]);
      Serial.print("-");
      Serial.print(rfid.uid.uidByte[2]);
      Serial.print("-");
      Serial.print(rfid.uid.uidByte[3]);
      Serial.print(", ");
      Serial.print("1");
      Serial.print(", "); 
      Serial.println("0");
      Serial.println();
    }
    //}

    
 // }else {
    //Serial.println(F("Card read previously."));
 // }
      
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  delay(800);




}
