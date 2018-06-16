#include "cardReader.h"

// Constructing MFRC522 object
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initializing constants for Keypad
const byte n_rows = 1;
const byte n_cols = 2;

char keys[n_rows][n_cols] = {
  {'2', '5'}
};

byte colPins[2] = {15, 3};
byte rowPins[n_rows] = {16};

// Constructing Keypad object
Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, n_rows, n_cols);

// Declaring global variables for string buffer and transaction value
char buffer[7];
double transactionValue;
bool newTransaction = true;

// Constructing LCD object
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup() {

  // Initializing LCD screen stuff
  lcd.begin(16, 2);
  lcd.init();
  lcd.setCursor(0, 0);

  // Turning backlight on
  lcd.backlight();

  // Initializing serial communication
  Serial.begin(9600);

  // Initializing SPI bus
  SPI.begin();

  lcd.setCursor(0, 0);
  lcd.print("Press p/ iniciar");
  keypad_read(25, 0, buffer, 1);
  lcd.clear();
  delay(2000);

  // Beginning wi-fi connection
  //WiFi.begin("Desktop_F5716385", "10203040");
  WiFi.begin("Jay Garrick", "TheSpeedForce");

  // Updating LCD screen content
  lcd.print("Conectando...");

  // Probably useless loop
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Printing IP address into LCD screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Endereco IP: ");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);
  lcd.clear();

  // Begin new transaction
  new_transaction();

}

void loop() {

  if (!newTransaction)
    return;

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  char uid[9];

  // Obtaining and checking TAG UID
  // If this returns false, transaction was cancelled by user
  if (!uid_read(uid)) {
    lcd.print("Passe a TAG RFID");
    return;
  }

  // Displaying tag/acc UID into the screen
  delay(2000);
  lcd.setCursor(0, 0);
  lcd.print("UID da Conta:");
  lcd.setCursor(0, 1);
  lcd.print(uid);
  delay(1000);
  lcd.clear();
  delay(1000);

  // Requesting password from user
  lcd.setCursor(0, 0);
  lcd.print("Senha:");

  // Reading password from keypad
  keypad_read(25, 0, buffer, 6, '\0', -1, '*');

  lcd.clear();

  delay(1000);

  lcd.print("Autenticando...");

  char api_token[71];

  // Attempting to perform login, actual transaction and then logout
  if (do_login(uid, buffer, api_token)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    delay(2000);
    Serial.println(api_token);

    if (do_transfer(api_token, transactionValue)) {
      if (do_logout(api_token)) {
        lcd.setCursor(0, 1);
        lcd.print("Sucesso");
      }
      else {
        lcd.setCursor(0, 1);
        lcd.print("Sucesso (!lgout)");
      }

      delay(5000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Nova transacao?");
      lcd.setCursor(0, 1);
      lcd.print("(2/5): ");
      keypad_read(25, 7, buffer, 1);
      lcd.clear();
      delay(2000);

      if (buffer[0] == '2') {
        new_transaction();
        return;
      }
      else {
        lcd.setCursor(0, 1);
        lcd.print("Fim");
        newTransaction = false;
        return;
      }
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print("Falha na transf");
    }
  }
  else {
    lcd.setCursor(0, 1);
    lcd.clear();
    lcd.print("Falha no login");
  }

  // Query user if he/she wants to redo the transaction in case of failure
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Refz transacao?");
  lcd.setCursor(0, 1);
  lcd.print("(2/5): ");
  keypad_read(25, 7, buffer, 1);
  lcd.clear();
  delay(2000);

  if (buffer[0] == '2') {
    lcd.print("Passe a TAG RFID");
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print("Fim");
    newTransaction = false;
  }
}

void new_transaction() {
  // Displaying message for value input in LCD screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Insira o valor:");

  lcd.setCursor(0, 1);
  lcd.print("DK ");
  int i = 0;

  // Reading input from keypad
  keypad_read(25, 3, buffer, 6, '.', 3);

  // Obtaining transaction value
  transactionValue = (double) atof(buffer);

  delay(100);

  // Initializing RFID reading
  mfrc522.PCD_Init();

  // Displaying message for requesting RFID tag from user
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Passe a TAG RFID");
}

void keypad_read(int delayTime, int startIndex, char* output, int size, char insertChar, int insertId, char replacementChar) {
  int i = 0;
  char myKey;

  while (i < size) {
    myKey = myKeypad.getKey();

    if (myKey != NULL) {
      lcd.setCursor(startIndex + i, 1);

      if (insertId != -1 && i == insertId) {
        lcd.print(insertChar);
        output[i] = insertChar;
        i++;
      }

      lcd.setCursor(startIndex + i, 1);

      if (replacementChar != '\0')
        lcd.print(replacementChar);
      else
        lcd.print(myKey);

      output[i] = myKey;

      i++;
      delay(delayTime);
    }
    else {
      delay(delayTime);
    }
  }

  output[i] = '\0';
}

bool uid_read(char* uid) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UID: ");
  lcd.setCursor(5, 0);

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    lcd.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    lcd.print(mfrc522.uid.uidByte[i], HEX);
    sprintf(&uid[i * 2], "%.2X", mfrc522.uid.uidByte[i]);
  }

  lcd.setCursor(0, 1);
  lcd.print("Confere? (2/5) ");
  keypad_read(25, 15, buffer, 1);
  delay(500);
  lcd.clear();

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  if (buffer[0] == '2')
    return true;
  else
    return false;
}

bool do_login(char* uid, char* pwd, char* api_token) {
  StaticJsonBuffer<400> jsonBuffer;

  char request_string[255];
  char output_string[511];
  JsonObject& root = jsonBuffer.createObject();
  root["rfid_key"] = uid;
  root["password"] = pwd;

  root.printTo(request_string);

  Serial.println(request_string);

  HTTPClient http;
  http.begin("http://mc857.viniciusfabri.com/api/login");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(request_string);

  Serial.println(httpCode);

  if (httpCode == 200) {
    String payload = http.getString();
    jsonBuffer.clear();

    JsonObject& output = jsonBuffer.parseObject(payload);
    Serial.println(payload);
    JsonObject& data = output["data"].as<JsonObject>();
    strcpy(api_token, data["api_token"].as<char*>());

    return true;
  }
  else {
    return false;
  }
}

bool do_logout(char* api_token) {
  StaticJsonBuffer<200> jsonBuffer;

  char request_string[255];
  JsonObject& root = jsonBuffer.createObject();
  root["api_token"] = api_token;

  root.printTo(request_string);
  Serial.println(request_string);

  HTTPClient http;
  http.begin("http://mc857.viniciusfabri.com/api/logout");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(request_string);
  Serial.println(httpCode);

  if (httpCode == 200)
    return true;
  else
    return false;

}

bool do_transfer(char* api_key, double transfer_amount) {
  StaticJsonBuffer<200> jsonBuffer;

  char request_string[255];
  JsonObject& root = jsonBuffer.createObject();
  root["api_token"] = api_key;
  root["to"] = "teste.chromiumos4@gmail.com";
  root["ammount"] = 100000000 * transfer_amount;

  root.printTo(request_string);
  Serial.println(request_string);

  HTTPClient http;
  http.begin("http://mc857.viniciusfabri.com/api/transfer");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(request_string);
  String payload = http.getString();
  Serial.println(payload);

  if (httpCode == 200)
    return true;
  else
    return false;

}


