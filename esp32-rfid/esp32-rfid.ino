#include <LiquidCrystal.h>
LiquidCrystal lcd(13, 12, 14, 27, 26, 25);

#include <WiFi.h>
#include <HTTPClient.h>

#include <MFRC522.h> //library responsible for communicating with the module RFID-RC522
#include <SPI.h> //library responsible for communicating of SPI bus
#define SS_PIN 21
#define RST_PIN 22
#define SIZE_BUFFER 18
#define MAX_SIZE_BLOCK 16
#define greenPin 12
#define redPin 32
//used in authentication
MFRC522::MIFARE_Key key;
//authentication return status code
MFRC522::StatusCode status;
// Defined pins to module RC522
MFRC522 mfrc522(SS_PIN, RST_PIN);

const char* ssid = "FAMILIA_VARGAS_BECERRA";
const char* password =  "ARcA19RuIN";

void setup()
{
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);

  // Init MFRC522
  mfrc522.PCD_Init();
  Serial.println("Approach your reader card...");
  Serial.println();

  lcd.begin(16, 2);
  // Clears The LCD Display
  lcd.clear();

  lcd.print("Hello World!");


  delay(4000);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  pinMode(4, OUTPUT);
}


void loop()
{
  // Aguarda a aproximacao do cartao
  //waiting the card approach
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select a card
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));</p><p> //call menu function and retrieve the desired option
  readingData();

  //instructs the PICC when in the ACTIVE state to go to a "STOP" state
  mfrc522.PICC_HaltA();
  // "stop" the encryption of the PCD, it must be called after communication with authentication, otherwise new communications can not be initiated
  mfrc522.PCD_StopCrypto1();
}


//reads data from card/tag
void readingData()
{
  //prints the technical details of the card/tag
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

  //prepare the key - all keys are set to FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //buffer for read data
  byte buffer[SIZE_BUFFER] = {0};

  //the block to operate
  byte block = 1;
  byte size = SIZE_BUFFER; //authenticates the block to operate
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    digitalWrite(redPin, HIGH);
    delay(1000);
    digitalWrite(redPin, LOW);
    return;
  }

  //read data from block
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    digitalWrite(redPin, HIGH);
    delay(1000);
    digitalWrite(redPin, LOW);
    return;
  }
  else {
    digitalWrite(greenPin, HIGH);
    delay(1000);
    digitalWrite(greenPin, LOW);
  }

  Serial.print(F("\nData from block ["));
  Serial.print(block); Serial.print(F("]: "));

  String id = "";

  //prints read data
  for (uint8_t i = 0; i < MAX_SIZE_BLOCK; i++)
  {
    Serial.write(buffer[i]);
    id += (char)buffer[i];
  }

  Serial.println(" ");

  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    HTTPClient http;

    http.begin("http://192.168.18.131:3000/" + id); //Specify the URL
    int httpCode = http.GET();                                        //Make the request

    if (httpCode == 200) { //Check for the returning code
      lcd.clear();
      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);

      lcd.setCursor(5, 1);
      // Display The Second Message In Position (5, 1)
      lcd.print("Bienvenido");

      digitalWrite(4, HIGH);
      delay(5000);
      digitalWrite(4, LOW);
    }
    else {
      lcd.clear();
      lcd.clear();
      Serial.println("Error on HTTP request");
      lcd.setCursor(0, 1);
      // Display The Second Message In Position (5, 1)
      lcd.print("Intente de nuevo");
      digitalWrite(4, HIGH);
      delay(200);
      digitalWrite(4, LOW);
      delay(200);
      digitalWrite(4, HIGH);
      delay(200);
      digitalWrite(4, LOW);
    }

    http.end(); //Free the resources
  }

}
