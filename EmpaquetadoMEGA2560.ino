#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 5
#define I2C_SLAVE_ADDR 0x04 //Dirección del esp32 esclavo (Enviador Wifi)
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
 
byte nuidPICC[4]; // Init array that will store new NUID

#include <MedianFilterLib2.h>
#include <movingAvg.h>
#include <Wire.h>
#include <VL53L1X.h>
#include <Arduino.h>
#include <WirePacker.h>

VL53L1X sensor;
MedianFilter2<int> medianFilter2(5);
MedianFilter2<int> medianFilterSub(5);
movingAvg avgTemp(5);

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

#define rojo 11   //Botones
#define negro 10

float distance;
float distfilt;
//float distEMA;
//float filtantEMA=0;
//float derivadaEMA=0;
float filtant=0;


#define sensorPin1 A1
#define sensorPin2 A2
int sensorValue1 = 0,sensorValue2 = 0;

bool arriba, abajo, nuevotag, empieza, reinicio, elige, select, abdominales, pushups, error1, error2, ejercitando;
int tiempoinicial=0, tiempofinal=0, abdos=0, abdosant=0, promedio, mediana, medianasub, pechadas, pechadasant,distsharp, promedioant = 0;
float derivada, deltaT = 0;
byte a=0, b=0, c=0, d=0, ej=0, kabs=0, kpech=0;




void setup() { 
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  Wire.begin();
  Wire.setClock(100000); // use 400 kHz I2C

  //sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1);
  }
  
  // Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
  // You can change these settings to adjust the performance of the sensor, but the minimum timing budget is 20 ms for short distance mode and 33 ms for
  // medium and long distance modes. See the VL53L1X datasheet for more information on range and timing limits.
  sensor.setDistanceMode(VL53L1X::Short);
  sensor.setMeasurementTimingBudget(50000);

  // Start continuous readings at a rate of one measurement every 50 ms (the inter-measurement period). This period should be at least as long as the timing budget.
  sensor.startContinuous(50);
  avgTemp.begin();  

  lcd.init();  // initialize the lcd 
  lcd.backlight(); 

  pinMode(rojo,INPUT);
  pinMode(negro,INPUT);
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);

  
}
 
void loop(){
  // Inicio del app
  if (reinicio==0){

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Acerque su TAG");
    
    abdos=0;
    abdosant=0;
    pechadas=0;
    pechadasant=0;
    reinicio=1;
    
  }
  // Registrar el TAG
  if (nuevotag==0){
    rfidRead();
  }

  // Elección de Ejercicio
  if (nuevotag==1&&elige==0){
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Rojo: Pechadas");
    lcd.setCursor(0,1);
    lcd.print("Negro: Abdominal");    
    elige=1;
  }
  // Elección de Ejercicio - Esperando respuesta de botones
  while(abdominales==0 && pushups==0 && nuevotag==1){
    if (digitalRead(negro)==HIGH){
      abdominales=1;
      pushups=0;
      error1=0;

      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("Empieza las");
      lcd.setCursor(3,1);
      lcd.print("abdominales");
      delay(1000);

    }
    if (digitalRead(rojo)==HIGH){
      abdominales=0;
      pushups=1;
      error1=0;

      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("Empieza las");
      lcd.setCursor(3,1);
      lcd.print("pechadas");
      delay(1000);

    }
    if ((digitalRead(11)==HIGH) && (digitalRead(10)==HIGH)){  //Detectar error si presiona ambos botones
      abdominales=0;
      pushups=0;
      if (error1==0){

      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Presiona de");
      lcd.setCursor(3,1);
      lcd.print("nuevo");
      delay(1000); 
      }
      error1=1;      
    }
    if ((digitalRead(11)==LOW) || (digitalRead(10)==LOW)){
      error1=0;      
    }
    ejercitando=1;
  }

  // Contador de Ejercicio
  while ((nuevotag==1) && (reinicio==1) && (ejercitando==1)){
    Contador();
    if (abdos!= abdosant){      
      enviador(rfid.uid.uidByte[0],rfid.uid.uidByte[1],rfid.uid.uidByte[2],rfid.uid.uidByte[3],1,(abdos-abdosant));    //Envia la info al esp32 que envia por Wifi cuando hay cambios
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Abdominales: ");
      lcd.setCursor(14,0);
      lcd.print(abdos);
      lcd.setCursor(0,1);
      lcd.print("Roj:Fin Neg:Otro");
    }
    if (pechadas!= pechadasant){
      enviador(rfid.uid.uidByte[0],rfid.uid.uidByte[1],rfid.uid.uidByte[2],rfid.uid.uidByte[3],2,(pechadas-pechadasant));
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Pechadas: ");
      lcd.setCursor(14,0);
      lcd.print(pechadas);
      lcd.setCursor(0,1);
      lcd.print("Roj:Fin Neg:Otro");
    }
   
    //Condición de Salida
    if (digitalRead(rojo)==HIGH){       //Termina el ejercicio
      nuevotag=0;
      empieza=0;
      reinicio=0;
      elige=0;
      select=0;
      error2=0;
      abdominales=0;
      pushups=0;
      ejercitando=0;
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Enviado");
      delay(1000);
     }
     if (digitalRead(negro)==HIGH){   //Vuelve para elegir otro ejercicio
      abdominales=0;
      pushups=0;
      elige=0;
      select=0;
      error2=0;
      ejercitando=0;
      delay(500);
    }
    if ((digitalRead(11)==HIGH) && (digitalRead(10)==HIGH)){          //Detectar error si presiona ambos botones
      if (error2==0){

      lcd.clear();
      lcd.setCursor(3,0);



















































































































































































      
      lcd.print("Presiona de");
      lcd.setCursor(3,1);
      lcd.print("nuevo");
      }
      error2=1;
      delay(300);      
    }
    if ((digitalRead(11)==LOW) || (digitalRead(10)==LOW)){
      error2=0;      
    }
     
  }

}


void rfidRead(){

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

//  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
//    rfid.uid.uidByte[1] != nuidPICC[1] || 
//    rfid.uid.uidByte[2] != nuidPICC[2] || 
//    rfid.uid.uidByte[3] != nuidPICC[3] ) {}
    
  nuevotag=1;
  rfid.PICC_HaltA(); // Halt PICC
  rfid.PCD_StopCrypto1(); // Stop encryption on PCD
}

void Contador(){
   
    //Obtención de distancia en mm
    distance=sensor.read();
    //Serial.println(distance);
    if (distance<=30 || distance >=500){ distance = 500; }//Fuera de Rango de Interés

    //Filtro Mediana móvil y Media móvil Opción 2
    mediana=medianFilter2.AddValue(distance);
    medianasub=medianFilterSub.AddValue(mediana);
    promedio = avgTemp.reading(mediana);

    distfilt=medianasub;
    Serial.print(distfilt);
    derivada=(distfilt-filtant);
    filtant=distfilt;
    Serial.print("\t");
    Serial.println(derivada);

    //Verificación de presión de pechadas
    sensorValue1 = analogRead(sensorPin1);
    sensorValue2 = analogRead(sensorPin2)*(3.0);

   //Contador de ejercicio
    if (derivada<(-20) && derivada>(-100) && distfilt<200){
      if (pushups==1){
        if (sensorValue1>(50) && sensorValue2>(50)){
            abajo=1;
        }
      }
      if (abdominales==1){
            abajo=1;
      }
      
    }
    
    if (derivada>(20) && derivada<(100) && abajo==1 && distfilt>380){
      if (pushups==1){
        if (sensorValue1>(50) && sensorValue2>(50)){
          arriba=1;
        }
      }
      if (abdominales==1){
          arriba=1;
      }

    }
    abdosant=abdos;
    pechadasant=pechadas;
    if (arriba==1 && abajo==1){
      if (abdominales==1){
        abdos+=1;
      }
      if (pushups==1){
        pechadas+=1;
      }
      arriba=0;
      abajo=0;
    }
}

void enviador(byte msg0, byte msg1, byte msg2, byte msg3, byte msg4, byte msg5){              //Función para enviar por I2C la info al ESP32 que enviará por Wifi
    WirePacker packer;        // first create a WirePacker that will assemble a packet
      packer.write(msg0);        // then add data the same way as you would with Wire
      packer.write(msg1);
      packer.write(msg2);        
      packer.write(msg3);
      packer.write(msg4);        
      packer.write(msg5);
    packer.end();             // after adding all data you want to send, close the packet

        Wire.beginTransmission(I2C_SLAVE_ADDR);  // now transmit the packed data
        while (packer.available()) {    // write every packet byte
            Wire.write(packer.read());
        }
        Wire.endTransmission();         // stop transmitting

}
