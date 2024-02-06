#include <Keypad.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>

#define FINGERPRINT_RX 16
#define FINGERPRINT_TX 17
#define RFID_SS_PIN 53
#define RFID_RST_PIN 5

SoftwareSerial fingerPrint(10, 11);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerPrint);

MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define rabin A8
#define enroll A0
#define del A1
#define up A2
#define down A3
#define openLight 3
#define closeLight 4
#define servoPin 5

char keys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[4] = {4, 5, 6, 7};
byte pin_column[4] = {8, 9, 10, 11};
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, 4, 4);

char tagUID[] = "19 47 64 C1";
char password[4];
boolean fingerprintAuthenticated = false;
boolean rfidAuthenticated = false;
boolean keypadAuthenticated = false;

void setup() {
  pinMode(enroll, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(del, INPUT_PULLUP);
  pinMode(rabin, INPUT_PULLUP);
  pinMode(openLight, OUTPUT);
  pinMode(closeLight, OUTPUT);
  
  lcd.begin(16, 2);
  finger.begin(57600);
  Serial.begin(9600);
  
  lcd.setCursor(0, 0);
  lcd.print("System Initializing");
  lcd.setCursor(0, 1);
  lcd.print("Please Wait...");
  delay(2000);

  lcd.clear();
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger or Tag");
  
  fingerprintAuthenticated = authenticateFingerprint();
  
  if (fingerprintAuthenticated) {
    openGate();
    delay(5000);
    closeGate();
    delay(2000);
  } else {
    lcd.clear();
    lcd.print("Fingerprint Failed");
    delay(2000);
    
    rfidAuthenticated = authenticateRFID();
    
    if (rfidAuthenticated) {
      openGate();
      delay(5000);
      closeGate();
      delay(2000);
    } else {
      lcd.clear();
      lcd.print("RFID Failed");
      delay(2000);
      
      keypadAuthenticated = authenticateKeypad();
      
      if (keypadAuthenticated) {
        openGate();
        delay(5000);
        closeGate();
        delay(2000);
      } else {
        lcd.clear();
        lcd.print("Authentication Failed");
        delay(2000);
      }
    }
  }
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();

  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Finger Not Found");
    lcd.setCursor(0, 1);
    lcd.print("Try Later");
    delay(2000);
    return -1;
  }

  // Found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  return finger.fingerID;
}

boolean authenticateFingerprint() {
  lcd.clear();
  lcd.print("Place Finger");
  delay(2000);
  int fingerprintID = getFingerprintIDez();
  
  if (fingerprintID >= 0) {
    lcd.clear();
    lcd.print("Fingerprint Authenticated");
    return true;
  } else {
    lcd.clear();
    lcd.print("Fingerprint Authentication Failed");
    return false;
  }
}

boolean authenticateRFID() {
  lcd.clear();
  lcd.print("Scan RFID Card");
  delay(2000);

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String tag = "";
    for (byte j = 0; j < mfrc522.uid.size; j++) {
      tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
      tag.concat(String(mfrc522.uid.uidByte[j], HEX));
    }
    tag.toUpperCase();
    
    if (tag == tagUID) {
      lcd.clear();
      lcd.print("RFID Authenticated");
      return true;
    } else {
      lcd.clear();
      lcd.print("RFID Authentication Failed");
      return false;
    }
  } else {
    lcd.clear();
    lcd.print("No RFID Card Detected");
    return false;
  }
}

boolean authenticateKeypad() {
  lcd.clear();
  lcd.print("Enter Password:");
  
  int i = 0;
  char key_pressed = 0;
  
  while (i < 4) {
    key_pressed = keypad.getKey();
    if (key_pressed) {
      password[i++] = key_pressed;
      lcd.setCursor(i - 1, 1);
      lcd.print('*');
      delay(200);
    }
  }

  lcd.clear();
  lcd.print("Password Entered");
  delay(2000);
  
  if (strncmp(password, "1234", 4) == 0) {
    lcd.clear();
    lcd.print("Keypad Authenticated");
    return true;
  } else {
    lcd.clear();
    lcd.print("Keypad Authentication Failed");
    return false;
  }
}

void openGate() {
  digitalWrite(openLight, HIGH);
  digitalWrite(closeLight, LOW);
  
  lcd.clear();
  lcd.print("Gate Opened");
   
  delay(5000);
}

void closeGate() {
  digitalWrite(openLight, LOW);
  digitalWrite(closeLight, HIGH);
  
  lcd.clear();
  lcd.print("Gate Closed");
    
  delay(2000);
}
