#include <LiquidCrystal_I2C.h>

#include <SPI.h>  
#include <MFRC522.h>  // Biblioteca do módulo RFID-RC522.
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Keypad.h>

#include <string.h>
#include <stdio.h>
#include <ArduinoJson.h>

#define SS_PIN 2  // Define o pino 10 como Escravo.
#define RST_PIN 0  // Define o pino 9 como Reset.

void new_transaction();
void keypad_read(int delayTime, int startIndex, char* output, int size, char insertChar = '\0', int insertId = -1, char replacementChar = '\0');
bool uid_read(char* uid);
bool do_login(char* uid, char* pwd, char* api_token);
bool do_logout(char* api_token);