# Smart Door Access System berbasis Arduino

## Deskripsi Program
Program ini adalah sistem kontrol akses pintu pintar (*Smart Door*) berbasis Arduino yang dirancang untuk memonitor dan membatasi jumlah orang di dalam suatu ruangan. Sistem ini menggunakan verifikasi ganda (Sensor IR dan Sensor Sentuh) untuk akses masuk, menghitung jumlah individu secara *real-time*, dan dilengkapi dengan fitur keselamatan (Mode Darurat) serta pengaman *timeout* untuk menutup pintu secara otomatis.

## Konfigurasi Pin Perangkat Keras

| Komponen | Pin Arduino | Tipe | Fungsi |
| :--- | :--- | :--- | :--- |
| **IR Luar** | 2 | Input | Mendeteksi orang di luar pintu |
| **IR Dalam** | 3 | Input | Mendeteksi orang di dalam ruangan |
| **Tombol Reset** | 4 | Input (Pullup) | Mengaktifkan/menonaktifkan Mode Darurat |
| **Touch Sensor** | 5 | Input | Verifikasi tambahan untuk akses masuk |
| **Buzzer** | 8 | Output | Indikator audio (akses diterima/ditolak) |
| **Motor Servo** | 9 | Output | Penggerak fisik pintu (Buka: 90°, Tutup: 0°) |
| **LED Hijau** | 10 | Output | Indikator akses diizinkan |
| **LED Merah** | 11 | Output | Indikator akses ditolak atau ruangan penuh |

---

## Fitur Utama

1. **Dual-Authentication Entry:** Akses masuk hanya terbuka jika sensor inframerah luar mendeteksi orang **dan** sensor sentuh diaktifkan secara bersamaan.
2. **Room Capacity Management:** Sistem membatasi jumlah orang di dalam ruangan (maksimal 3 orang). Jika kapasitas penuh, sistem akan menolak akses masuk baru.
3. **Emergency Override:** Tombol fisik untuk membuka pintu secara paksa dalam kondisi darurat, ditandai dengan alarm buzzer terus-menerus dan kedua LED menyala.
4. **Auto-Close Timeout:** Jika pintu terbuka selama lebih dari 10 detik dan tidak ada orang yang melewati sensor konfirmasi (IR Dalam untuk masuk / IR Luar untuk keluar), pintu akan tertutup secara otomatis untuk mencegah akses tidak sah.
5. **Anti-Spam Serial Monitor:** Implementasi variabel *state* (`lastStatus`) untuk memastikan pesan di Serial Monitor hanya dicetak saat ada perubahan status.

---

## Cara Kerja dan Alur Program

Program berjalan menggunakan metode *polling* di dalam fungsi `loop()` untuk terus memeriksa status sensor dan mengeksekusi fungsi yang sesuai.

### 1. Alur Akses Masuk (`accessEntry` & `checkEntry`)
* **Inisiasi:** Seseorang di luar ruangan terdeteksi oleh **IR Luar** dan menyentuh **Touch Sensor**.
* **Validasi Kapasitas:** Sistem mengecek apakah `jmlOrg < maxOrg`. 
  * Jika penuh, pintu tetap tertutup, LED Merah menyala, dan akses ditolak.
  * Jika belum penuh, pintu terbuka (Servo 90°), LED Hijau menyala, dan buzzer berbunyi singkat.
* **Konfirmasi:** Sistem masuk ke status `waitingMasuk`. Pintu akan tetap terbuka sampai **IR Dalam** mendeteksi orang tersebut telah melewati pintu.
* **Penyelesaian:** Setelah IR Dalam terpicu, jumlah orang (`jmlOrg`) bertambah 1, pintu kembali ditutup (Servo 0°), dan status dikembalikan ke normal.

### 2. Alur Akses Keluar (`accessExit` & `checkExit`)
* **Jeda Aman:** Terdapat `exitDelay` selama 2 detik setelah seseorang masuk sebelum sistem mengizinkan akses keluar, mencegah pemicuan sensor ganda secara tidak sengaja.
* **Inisiasi:** **IR Dalam** mendeteksi seseorang yang ingin keluar (tidak memerlukan sensor sentuh).
* **Eksekusi:** Pintu terbuka, LED Hijau menyala, dan buzzer berbunyi dengan nada yang sedikit berbeda (1200 Hz). Sistem masuk ke status `waitingKeluar`.
* **Konfirmasi:** Setelah **IR Luar** mendeteksi orang tersebut telah melewati pintu, jumlah orang (`jmlOrg`) berkurang 1, pintu ditutup, dan status dikembalikan ke normal.

### 3. Mode Darurat (`cekEmergency` & `modeEmergency`)
* Sistem secara aktif membaca status `resetBtn` menggunakan logika *state-change detection*.
* Jika tombol ditekan, status `emergencyMode` berubah menjadi aktif.
* **Perilaku Darurat:** Seluruh logika akses diabaikan. Pintu langsung terbuka (Servo 90°), LED Merah dan Hijau menyala bersamaan, dan Buzzer berbunyi panjang (1000 Hz) tanpa henti sampai tombol ditekan kembali untuk menonaktifkan mode ini.

### 4. Timeout Pintu (`checkTimeout`)
* Saat pintu terbuka (baik untuk masuk maupun keluar), sistem mencatat waktu buka pada variabel `openTime` menggunakan fungsi `millis()`.
* Jika pintu sudah terbuka melebihi durasi `timeoutDoor` (10.000 ms atau 10 detik) dan sensor konfirmasi belum terpicu, sistem akan membatalkan status `waiting` dan memaksa pintu untuk ditutup.
