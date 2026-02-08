#include <WiFi.h>
#include <HTTPClient.h>

// ---- WiFi Ayarları ----
const char* ssid = "**********";
const char* password = "*********";

// ---- ThingSpeak Ayarları ----
String apiKey = "*********";   // Write API Key
const char* server = "http://api.thingspeak.com/update";

// ---- Değişkenler ----
int skorA = 0;
int skorB = 0;
int dakika = 0;
int saniye = 0; 

// UART Ayarları (Serial2)
#define UART_RX 26   // STM32 TX -> ESP32 RX
#define UART_TX 25   // STM32 RX -> ESP32 TX
#define UART_BAUD 9600

void setup() {
  Serial.begin(115200);  // USB tarafı (Serial Monitor)
  delay(1000);
  
  // UART2 başlat (STM32 ile haberleşme)
  Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);

  Serial.println("Sistem Baslatiliyor...");

  // WiFi bağlantısı
  WiFi.begin(ssid, password);                  
  Serial.print("WiFi baglaniyor");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi baglandi!");
}

void loop() { 

  // 1. STM32'den veri geldi mi?
  if (Serial2.available()) {
    
    // Satır sonuna kadar oku
    String gelenVeri = Serial2.readStringUntil('\n');
    gelenVeri.trim(); // Boşlukları temizle

    if (gelenVeri.length() > 0) {
      Serial.print("Gelen Ham Veri: ");
      Serial.println(gelenVeri);

      // 2. PARSE İŞLEMİ (Saniye dahil)
      // Format: "A:3,B:1,D:15,S:30"
      
      int n = sscanf(gelenVeri.c_str(), "A:%d,B:%d,D:%d,S:%d", &skorA, &skorB, &dakika, &saniye);

      if (n == 4) { // 4 değişkenin dördünü de bulduysa
        Serial.printf("Veri Cozuldü -> A:%d B:%d Dk:%d Sn:%d\n", skorA, skorB, dakika, saniye);
        
        // 3. ThingSpeak'e Gönder
        sendToThingSpeak(skorA, skorB, dakika, saniye);
      } 
      else {
        Serial.println("Format Hatasi! Beklenen: A:x,B:y,D:z,S:t");
      }
    }
  }
}

void sendToThingSpeak(int sA, int sB, int dk, int sn) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // URL Oluşturma (4 Field birden gönder)
    String postData = "api_key=" + apiKey + 
                      "&field1=" + String(sA) + 
                      "&field2=" + String(sB) + 
                      "&field3=" + String(dk) +
                      "&field4=" + String(sn);

    int httpCode = http.POST(postData);

    if (httpCode > 0) {
      Serial.printf("ThingSpeak Gonderildi (Kod: %d)\n", httpCode);
    } else {
      Serial.printf("ThingSpeak Hatasi (Kod: %d)\n", httpCode);
    }
    http.end();
    
    // ThingSpeak'in ücretsiz sürümü 15 saniyede bir veri kabul eder.
    // STM32 çok hızlı veri yolluyorsa burada bekletmek iyidir.
    delay(15000); 
  } else {
    Serial.println("Internet baglantisi koptu!");
}
}
