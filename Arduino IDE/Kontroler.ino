#include <WiFi.h>
#include <esp_now.h>


uint8_t autoMAC[] = {0x30, 0xAE, 0xA4, 0xF8, 0x11, 0xB0};

typedef struct {
  int gas;
  int volan;
} Poruka;

Poruka podatak;


void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW greska");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, autoMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Peer greska");
    return;
  }

  Serial.println("Kontroler spreman");
}


void loop() {

  int x = analogRead(34);  // gas
  int y = analogRead(32);  // volan


  podatak.gas = x;
  podatak.volan = y;


  esp_now_send(autoMAC, (uint8_t *)&podatak, sizeof(podatak));


  Serial.print("Gas: ");
  Serial.print(podatak.gas);

  Serial.print("  Volan: ");
  Serial.println(podatak.volan);


  delay(50);
}