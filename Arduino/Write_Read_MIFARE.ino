/**
 * 
 *  Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 */
 #include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

 void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
  //Serial.print(F("Leer y escribir tarjeta MIFARE."));
 }
 

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
        return;
  // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;
   //Serial.print(F("Card UID:"));
   //dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
   //Serial.println();
    //Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    //Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return;
    }

    byte buffer[18];
    byte buffer2[18]; 
    byte buffer3[18];
    byte block;
    MFRC522::StatusCode status;
    byte len;
    byte sector = 1;
    byte blockAddr = 5; //Se especifica que bloque se leera 
    byte size = sizeof(buffer);


    //Escritura de bloque 5    
    Serial.setTimeout(100L) ;
    // Preguntar Por el ID
    //Serial.println(F("Ingrese el ID de la tarjeta, Finaliza con: #"));
    len = Serial.readBytesUntil('#', (char *) buffer, 16) ;
    
    for (byte i = len; i < 16; i++) {
      buffer[i] = ' ';     // Espacios en blanco
    }
 
    for(int i = 0; i < size; i++) {
     buffer2[i] = (buffer[i]-48);
    }

     buffer3[0] = (buffer2[0]*100)+(buffer2[1]*10)+buffer2[2];
     buffer3[1] = (buffer2[4]*100)+(buffer2[5]*10)+buffer2[6];
     buffer3[2] = (buffer2[8]*100)+(buffer2[9]*10)+buffer2[10];
    
    for(int i = 3; i < 18; i++) {      
      buffer3[i] = 0x00;       
    }

    block = 5;
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));

    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() falla: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    // Escribir un bloque
    
    status = mfrc522.MIFARE_Write(block, buffer3, 16);
    
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() falla: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    // Read data from the block
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    
    dump_byte_array(buffer, 3);
    Serial.println();
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
}


void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], DEC);
    }
}

