# 🏟️ STM32 IoT Tabanlı Akıllı Skor Tablosu Projesi

Bu proje, **STM32G030C8T6 (DemeDU Kit)** mikrodenetleyicisi kullanılarak geliştirilmiş; maç sürelerini, skorları ve çevre koşullarını takip eden, verileri anlık olarak **ThingSpeak** bulut platformuna aktaran kapsamlı bir gömülü sistem projesidir.

---

## 🚀 Öne Çıkan Özellikler

* **Hassas Kronometre:** `Timer3` kesmesi kullanılarak milisaniyelik doğrulukla 2 devreli maç süresi takibi.
* **Akıllı Stadyum Işıkları:** `ADC` üzerinden okunan LDR verisi ile ortam karardığında otomatik yanan aydınlatma sistemi.
* **İnteraktif Kontrol:** 3 adet buton (`EXTI` kesmesi) ile maç başlatma/durdurma, takım seçimi ve skor yönetimi.
* **Gelişmiş Ses/Işık Geri Bildirimi:** * Gol anında özel "Gol Sesi" efekti.
    * Maç sonunda hakem düdüğü simülasyonu.
    * Maç başı ve sonu için RGB LED animasyon dizilimleri.
* **IoT Entegrasyonu:** `UART` haberleşmesi üzerinden skor ve süre bilgilerinin buluta (ThingSpeak) aktarılması.

---

## 🛠️ Donanım Mimarisi

* **MCU:** STM32G030C8T6 (ARM® Cortex®-M0+)
* **Haberleşme:** ESP8266 WiFi Modülü & USB-to-TTL Köprüsü
* **Sensörler:** LDR (Işık Sensörü - ADC Kanal 5)
* **Çıkışlar:** Buzzer, RGB LED, Çoklu Durum LED'leri
* **Girişler:** 3x Push Button (Pull-up konfigürasyonu)

---

## 📡 IoT ve Bulut Veri Akışı

Proje, verileri ThingSpeak platformuna aktarmak için hibrit bir donanım yapılandırması kullanır:

1. **Veri Paketleme:** STM32, güncel skoru ve süreyi `A:skor,B:skor,D:dk,S:sn` formatında paketler.
2. **Haberleşme Köprüsü:** Kart üzerindeki mini jumperlar ESP bacaklarına yönlendirilir ve USB-to-TTL üzerinden ESP'ye veri aktarılır.
3. **Bulut Aktarımı:** ESP modülü içerisindeki Arduino yazılımı, gelen veriyi yakalar ve WiFi üzerinden ThingSpeak API'sine gönderir.

<p align="center">
  <img src="./thingspeak_veriler.jpeg" width="700" title="ThingSpeak Veri Analizi">
  <br>
  <i>Görsel 1: ThingSpeak üzerinden anlık veri takibi ve skor analizi paneli</i>
</p>

---

## 💻 Yazılım Detayları

Proje, **STM32CubeIDE** ortamında `HAL` kütüphaneleri kullanılarak geliştirilmiştir.

* **Kesme (Interrupt) Yönetimi:** Buton arkı (debounce) engelleme algoritmaları ve zamanlayıcı kesmeleri senkronize çalışır.
* **Güç Yönetimi:** Maç durduğunda veya bittiğinde donanım birimleri (LED/Buzzer) otomatik olarak düşük güç/bekleme moduna geçer.

---

## 🔧 Kullanım Talimatları

1. **Kod Yükleme:** Debugger portu üzerinden projeyi karta yükleyin.
2. **Mod Değişimi:** Yükleme sonrası jumper ayarlarını ESP modülüne yönlendirin.
3. **Ağ Bağlantısı:** ESP modülünün telefonunuzun internetine (Hotspot) bağlı olduğundan emin olun.
4. **İzleme:** ThingSpeak kanalı üzerinden maç istatistiklerini anlık olarak görüntüleyin.

---

**Geliştiren:** Yunus Kunduz  
**Mühendislik Derslerine Ulaşmak İçin:** [CozumLab](https://www.youtube.com/@CozumLabTR)  
*Bu proje Gömülü Sistem Tasarımı dersi kapsamında mühendislik çözümü olarak üretilmiştir.*
