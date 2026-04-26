#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MPU6050.h>
#include <Servo.h>

// ==========================
// OBJECT
// ==========================
LiquidCrystal_I2C lcd(0x27, 20, 4);
MPU6050 mpu;

Servo servoPitch;
Servo servoRoll;

// ==========================
// PIN
// ==========================
#define BTN 2
#define JOY_X A0
#define JOY_Y A1
#define SERVO_PITCH 9
#define SERVO_ROLL 10

// ==========================
// STATE
// ==========================
volatile bool changeModeRequest = false;
bool modeAuto = true;

bool sedangTransisi = false;
unsigned long waktuTransisi = 0;
int stepTransisi = 0;

// ==========================
// CONTROL PARAM
// ==========================
float gainGyro = 0.5;
float alpha = 0.2;

float pitchRateFiltered = 0;
float rollRateFiltered = 0;

// ==========================
// INTERRUPT
// ==========================
void gantiMode() {
  if (!sedangTransisi) {
    changeModeRequest = true;
  }
}

// ==========================
// FORMAT OFFSET
// ==========================
String formatOffset(int val) {
  char buffer[6];
  sprintf(buffer, "%+04d", val);
  return String(buffer);
}

// ==========================
// HEADER MODE
// ==========================
void tampilkanMode() {
  String text;

  if (modeAuto) {
    text = "[AUTO] MANUAL";
  } else {
    text = "AUTO [MANUAL]";
  }

  int pos = (20 - text.length()) / 2;
  lcd.setCursor(pos, 0);
  lcd.print(text);
}

// ==========================
void setup() {
  //inisiasi serial communication
  Serial.begin(9600);

  //inisialisasi LCD
  lcd.init();
  lcd.backlight();

  //inisialisasi MPU6050
  Wire.begin();
  mpu.initialize();

  //inisialisasi servo
  servoPitch.attach(SERVO_PITCH);
  servoRoll.attach(SERVO_ROLL);

  //inisialisasi tombol dan interrupt
  pinMode(BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN), gantiMode, FALLING);

  //set posisi awal servo ke tengah
  servoPitch.write(90);
  servoRoll.write(90);

  //memunculkan header di lcd
  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("SMART GIMBAL");
  delay(1000);
}

// ==========================
void loop() {

  //cek interrupt untuk ganti mode
  if (changeModeRequest && !sedangTransisi) {
    sedangTransisi = true;
    stepTransisi = 0;
    waktuTransisi = millis();
    changeModeRequest = false;
  }

  //jika sedangTransisi true, jalankan proses transisi
  if (sedangTransisi) {
    prosesTransisi();
    return;
  }

  //switch mode otomatis atau manual
  if (modeAuto) {
    modeOtomatis();
  } else {
    modeManual();
  }

  delay(20);
}

// ==========================
// TRANSISI MODE
// ==========================
void prosesTransisi() {

  // setiap 500ms, lakukan step berikutnya
  unsigned long now = millis();

  // jeda semua input selama transisi
  if (stepTransisi == 0 && now - waktuTransisi >= 500) {
    waktuTransisi = now;
    stepTransisi++;
  }

  // reset posisi servo pitch ke tengah
  else if (stepTransisi == 1 && now - waktuTransisi >= 500) {
    servoPitch.write(90);
    waktuTransisi = now;
    stepTransisi++;
  }

  // reset posisi servo roll ke tengah
  else if (stepTransisi == 2 && now - waktuTransisi >= 500) {
    servoRoll.write(90);
    waktuTransisi = now;
    stepTransisi++;
  }

  // ubah variabel mode setelah semua servo sudah direset
  else if (stepTransisi == 3 && now - waktuTransisi >= 500) {
    modeAuto = !modeAuto;
    waktuTransisi = now;
    stepTransisi++;
  }

  // kembalikan variabel sedangTransisi ke false setelah semua step selesai
  else if (stepTransisi == 4 && now - waktuTransisi >= 500) {
    sedangTransisi = false;
  }

  //tampilkan pesan transisi di LCD
  lcd.clear();
  tampilkanMode();
  lcd.setCursor(3,2);
  lcd.print("SWITCHING...");
}

// ==========================
// MODE AUTO (GYRO ONLY)
// ==========================
void modeOtomatis() {

  // baca data gyro
  int16_t gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);

  // konversi ke derajat per detik
  float pitchRate = gx / 131.0;
  float rollRate  = gy / 131.0;

  // smoothing dengan low-pass filter
  pitchRateFiltered = (1 - alpha) * pitchRateFiltered + alpha * pitchRate;
  rollRateFiltered  = (1 - alpha) * rollRateFiltered  + alpha * rollRate;

  // update posisi servo berdasarkan gyro
  int currentP = servoPitch.read();
  int currentR = servoRoll.read();

  // gunakan threshold untuk menghindari noise kecil
  if (abs(pitchRateFiltered) > 1) {
    currentP += pitchRateFiltered * gainGyro;
  }

  if (abs(rollRateFiltered) > 1) {
    currentR += rollRateFiltered * gainGyro;
  }


  // batasi posisi servo antara 0-180
  currentP = constrain(currentP, 0, 180);
  currentR = constrain(currentR, 0, 180);

  // update posisi servo
  servoPitch.write(currentP);
  servoRoll.write(currentR);

  // LCD
  lcd.clear();

  tampilkanMode();

  lcd.setCursor(0,1);
  lcd.print("Pitch:");
  lcd.print(pitchRateFiltered);

  lcd.setCursor(0,2);
  lcd.print("Roll :");
  lcd.print(rollRateFiltered);

  // tampilkan offset dari posisi tengah (90)
  int offsetP = currentP - 90;
  int offsetR = currentR - 90;

  lcd.setCursor(0,3);
  lcd.print("S0:");
  lcd.print(formatOffset(offsetP));
  lcd.print(" S1:");
  lcd.print(formatOffset(offsetR));
}

// ==========================
// MODE MANUAL
// ==========================
void modeManual() {

  // baca data joystick
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  // baca posisi servo saat ini
  int posP = servoPitch.read();
  int posR = servoRoll.read();

  // deadzone sebagai treshold noise
  int deadzone = 50; // sekitar tengah (512)

  // hitung perubahan posisi berdasarkan joystick
  int deltaX = 0;
  int deltaY = 0;

  // gunakan map untuk mengubah range 0-1023 menjadi -3 sampai 3 (kecepatan gerak)
  if (abs(x - 512) > deadzone) {
    deltaX = map(x, 0, 1023, -3, 3);
  }

  if (abs(y - 512) > deadzone) {
    deltaY = map(y, 0, 1023, -3, 3);
  }

  // ===== KONTROL =====
  if (deltaX != 0 || deltaY != 0) {
    // jika ada input servo bergerak sesuai input joystick
    posP += deltaY;
    posR += deltaX;
  } else {
    // jika tidak ada input posisi servo balik ke tengah
    posP += (90 - posP) * 0.05;
    posR += (90 - posR) * 0.05;
  }

  // batasi posisi servo antara 0-180
  posP = constrain(posP, 0, 180);
  posR = constrain(posR, 0, 180);

  // update posisi servo
  servoPitch.write(posP);
  servoRoll.write(posR);

  // ===== LCD =====
  lcd.clear();
  tampilkanMode();

  lcd.setCursor(0,1);
  lcd.print("Joy X:");
  lcd.print(x);

  lcd.setCursor(0,2);
  lcd.print("Joy Y:");
  lcd.print(y);

  // tampilkan offset dari posisi tengah (90)
  int offsetP = posP - 90;
  int offsetR = posR - 90;

  // format offset dengan tanda + atau - dan 4 digit (misal +0050 atau -0030)
  String row4 = "S0:" + formatOffset(offsetP) + " S1:" + formatOffset(offsetR);
  int pos = (20 - row4.length()) / 2;

  lcd.setCursor(pos, 3);
  lcd.print(row4);
}