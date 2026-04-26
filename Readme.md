# Gimbal Gambil (Project I Kelompok 6)

```
Rizki Arif Saifudin     H1D023067
Khafriza Diaz Permana   H1D023072
Muhammad Zaim Pahlevi   H1D023073
Daiva Paundra Gevano    H1D023075
Aditya Fathan Naufaldi  H1D023076
```

## 📌 Deskripsi

Proyek ini merupakan implementasi sistem gimbal sederhana berbasis Arduino Uno yang mampu menjaga stabilitas sudut menggunakan sensor gyro serta menyediakan kontrol manual menggunakan joystick.

Sistem memiliki dua mode utama:

* **Mode Otomatis** → menggunakan sensor MPU6050 untuk stabilisasi
* **Mode Manual** → dikontrol menggunakan joystick

Simulasi dibuat menggunakan platform Wokwi sehingga dapat dijalankan tanpa perangkat fisik.

---

## ⚙️ Komponen yang Digunakan

* Arduino Uno
* MPU6050 (Gyroscope & Accelerometer)
* LCD 20x4 (I2C)
* 2x Servo Motor
* Analog Joystick
* Push Button

---

## 🔌 Konfigurasi Pin

| Komponen    | Pin Arduino |
| ----------- | ----------- |
| MPU6050 SDA | A4          |
| MPU6050 SCL | A5          |
| LCD SDA     | A4          |
| LCD SCL     | A5          |
| Joystick X  | A0          |
| Joystick Y  | A1          |
| Servo Pitch | 9           |
| Servo Roll  | 10          |
| Button      | 2           |

---

## 🧠 Cara Kerja Sistem

### 1. Mode Otomatis

Sistem membaca data rotasi dari MPU6050 (gyro), kemudian:

* Data di-filter menggunakan **low-pass filter**
* Diberikan **deadzone** untuk menghindari noise kecil
* Nilai digunakan untuk menggerakkan servo agar stabil

### 2. Mode Manual

* Input joystick dibaca dari A0 dan A1
* Nilai dipetakan menjadi pergerakan servo
* Jika tidak ada input, servo kembali ke posisi tengah secara smooth

### 3. Pergantian Mode

* Menggunakan push button (interrupt pada pin 2)
* Terdapat animasi transisi (servo kembali ke tengah sebelum mode berubah)

---

## 🖥️ Tampilan LCD

LCD menampilkan:

* Mode aktif (AUTO / MANUAL)
* Nilai sensor (pitch & roll)
* Nilai joystick
* Offset posisi servo

---

## ▶️ Link Wokwi dan Video Demo

Wokwi      : https://wokwi.com/projects/461152748931069953
Video Demo : https://youtu.be/zznkCxK1MpA

---

## 🔧 Fitur Utama

* Dual mode (Auto & Manual)
* Filtering data gyro (low-pass filter)
* Deadzone untuk mengurangi noise
* Smooth return ke posisi tengah
* Interrupt-based mode switching
* Tampilan real-time di LCD
