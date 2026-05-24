#include <Servo.h>

// ======================= PIN CONFIG =======================
const int irLuar   = 2;
const int irDalam  = 3;
const int resetBtn = 4;
const int touchPin = 5;

const int buzzer   = 8;
const int servoPin = 9;
const int ledR     = 11;
const int ledG     = 10;

// ======================= SERVO ============================
Servo pintu;

// ======================= SYSTEM ===========================
const int maxOrg = 3;
int jmlOrg = 0;

bool emergencyMode = false;

bool waitingMasuk  = false;
bool waitingKeluar = false;

unsigned long openTime = 0;
const unsigned long timeoutDoor = 10000;

unsigned long lastEntryTime = 0;
const unsigned long exitDelay = 2000;

// Untuk mencegah spam Serial Monitor
String lastStatus = "";

// ==========================================================
// SETUP
// ==========================================================

void setup() {
  Serial.begin(9600);

  pinMode(irLuar, INPUT);
  pinMode(irDalam, INPUT);

  pinMode(resetBtn, INPUT_PULLUP);
  pinMode(touchPin, INPUT);

  pinMode(buzzer, OUTPUT);

  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);

  pintu.attach(servoPin);

  closeDoor();

  printStatus("=== SMART DOOR ACCESS READY ===");
}

// ==========================================================
// LOOP
// ==========================================================

void loop() {

  cekEmergency();

  // ================= EMERGENCY MODE =================
  if (emergencyMode) {
    modeEmergency();
    return;
  }

  // ================= AKSES MASUK =================
  if (jmlOrg < maxOrg) {
    accessEntry();
  }
  else {
    roomFullMode();
  }

  // ================= AKSES KELUAR =================
  accessExit();

  // ================= CHECK SENSOR =================
  checkEntry();
  checkExit();

  // ================= TIMEOUT =================
  checkTimeout();
}

// ==========================================================
// EMERGENCY BUTTON
// ==========================================================

void cekEmergency() {

  static bool lastButtonState = HIGH;

  bool buttonState = digitalRead(resetBtn);

  if (buttonState == LOW && lastButtonState == HIGH) {

    emergencyMode = !emergencyMode;

    delay(200);

    if (emergencyMode) {
      printStatus("!!! EMERGENCY MODE AKTIF !!!");
    }
    else {
      printStatus("Emergency Mode Nonaktif");
      closeDoor();
    }
  }

  lastButtonState = buttonState;
}

// ==========================================================
// EMERGENCY MODE
// ==========================================================

void modeEmergency() {

  pintu.write(90);

  digitalWrite(ledR, HIGH);
  digitalWrite(ledG, HIGH);

  tone(buzzer, 1000);

  // Tidak spam serial
}

// ==========================================================
// MODE RUANGAN PENUH
// ==========================================================

void roomFullMode() {

  digitalWrite(ledR, HIGH);
  digitalWrite(ledG, LOW);

  printStatus("Ruangan Penuh - Akses Masuk Ditolak");
}

// ==========================================================
// AKSES MASUK
// ==========================================================

void accessEntry() {

  bool luarDetected  = digitalRead(irLuar) == LOW;
  bool touchDetected = digitalRead(touchPin) == HIGH;

  // Orang ingin masuk
  if (luarDetected && touchDetected && !waitingMasuk && !waitingKeluar) {

    waitingMasuk = true;

    openDoor();

    digitalWrite(ledG, HIGH);
    digitalWrite(ledR, LOW);

    tone(buzzer, 1000);
    delay(200);
    noTone(buzzer);

    openTime = millis();

    printStatus("Akses Masuk Diterima");
  }

  // Akses ditolak
  else if (luarDetected && !touchDetected && !waitingMasuk) {

    digitalWrite(ledR, HIGH);
    digitalWrite(ledG, LOW);

    printStatus("Akses Ditolak - Sentuh Sensor");
  }
}

// ==========================================================
// CEK ORANG MASUK
// ==========================================================

void checkEntry() {

  if (!waitingMasuk) {
    return;
  }

  bool dalamDetected = digitalRead(irDalam) == LOW;

  if (dalamDetected) {

    jmlOrg++;

    printJumlah();

    closeDoor();

    waitingMasuk = false;
    
    lastEntryTime = millis();

    delay(500);
  }
}

// ==========================================================
// AKSES KELUAR
// ==========================================================

void accessExit() {

  if (millis() - lastEntryTime < exitDelay) {
    return;
  }

  bool dalamDetected = digitalRead(irDalam) == LOW;

  // Orang dari dalam ingin keluar
  if (dalamDetected && !waitingKeluar && !waitingMasuk && jmlOrg > 0) {

    waitingKeluar = true;

    openDoor();

    digitalWrite(ledG, HIGH);
    digitalWrite(ledR, LOW);

    tone(buzzer, 1200);
    delay(200);
    noTone(buzzer);

    openTime = millis();

    printStatus("Akses Keluar");
  }
}

// ==========================================================
// CEK ORANG KELUAR
// ==========================================================

void checkExit() {

  if (!waitingKeluar) {
    return;
  }

  bool luarDetected = digitalRead(irLuar) == LOW;

  if (luarDetected) {

    if (jmlOrg > 0) {
      jmlOrg--;
    }

    printJumlah();

    closeDoor();

    waitingKeluar = false;

    delay(500);
  }
}

// ==========================================================
// TIMEOUT PINTU
// ==========================================================

void checkTimeout() {

  if ((waitingMasuk || waitingKeluar) &&
      millis() - openTime >= timeoutDoor) {

    printStatus("Timeout - Pintu Ditutup");

    closeDoor();

    waitingMasuk  = false;
    waitingKeluar = false;
  }
}

// ==========================================================
// BUKA PINTU
// ==========================================================

void openDoor() {

  pintu.write(90);

  printStatus("Pintu Terbuka");
}

// ==========================================================
// TUTUP PINTU
// ==========================================================

void closeDoor() {

  pintu.write(0);

  digitalWrite(ledR, LOW);
  digitalWrite(ledG, LOW);

  noTone(buzzer);

  printStatus("Pintu Tertutup");
}

// ==========================================================
// PRINT JUMLAH ORANG
// ==========================================================

void printJumlah() {

  Serial.print("Jumlah Orang di Dalam: ");
  Serial.println(jmlOrg);
}

// ==========================================================
// ANTI LOOP SERIAL MONITOR
// ==========================================================

void printStatus(String msg) {

  if (msg != lastStatus) {

    Serial.println(msg);

    lastStatus = msg;
  }
}