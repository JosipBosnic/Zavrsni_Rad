#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

// ==================================================
// SERVO VOLANA
// ==================================================

Servo volan;

int trenutniKut = 90;
int ciljaniKut = 90;


// ==================================================
// ŽMIGAVCI
// ==================================================

const int lijeviZmigavac = 21;
const int desniZmigavac = 22;

unsigned long zadnjeTrepere = 0;
bool stanjeTrepere = false;

const int vrijemeTrepere = 400;


// ==================================================
// ESP-NOW PORUKA
// ==================================================

typedef struct {
  int gas;
  int volan;
} Poruka;

Poruka podatak;


// ==================================================
// MOTOR
// ==================================================

void motor(int smjer) {

  if (smjer == 1) {

    digitalWrite(17, HIGH);
    digitalWrite(18, LOW);
  }

  else if (smjer == -1) {

    digitalWrite(17, LOW);
    digitalWrite(18, HIGH);
  }

  else {

    digitalWrite(17, LOW);
    digitalWrite(18, LOW);
  }
}


// ==================================================
// UGASI ŽMIGAVCE
// ==================================================

void ugasiZmigavce() {

  digitalWrite(lijeviZmigavac, LOW);
  digitalWrite(desniZmigavac, LOW);
}


// ==================================================
// TREPERENJE ŽMIGAVACA
// ==================================================

void treperiZmigavac(int lijevi, int desni) {

  unsigned long trenutnoVrijeme = millis();

  if (trenutnoVrijeme - zadnjeTrepere >= vrijemeTrepere) {

    zadnjeTrepere = trenutnoVrijeme;

    stanjeTrepere = !stanjeTrepere;
  }

  if (stanjeTrepere) {

    digitalWrite(lijeviZmigavac, lijevi);
    digitalWrite(desniZmigavac, desni);
  }

  else {

    digitalWrite(lijeviZmigavac, LOW);
    digitalWrite(desniZmigavac, LOW);
  }
}


// ==================================================
// PRIMANJE ESP-NOW PODATAKA
// ==================================================

void primiPodatak(
  const esp_now_recv_info *info,
  const uint8_t *incomingData,
  int len
) {

  memcpy(&podatak, incomingData, sizeof(podatak));


  // ==================================================
  // VOLAN
  // ==================================================

  int vrijednostVolana = podatak.volan;


  // Mrtva zona volana
  // Stvarna sredina je oko 1865

  if (vrijednostVolana >= 1765 &&
      vrijednostVolana <= 1965) {

    vrijednostVolana = 1865;
  }


  // Joystick 0-4095 -> servo 45-135 stupnjeva

  int kut = map(
    vrijednostVolana,
    0,
    4095,
    45,
    135
  );


  kut = constrain(kut, 45, 135);


  // Postavi ciljani kut

  ciljaniKut = kut;


  // ==================================================
  // GAS
  // ==================================================

  if (podatak.gas > 2500) {

    motor(1);
  }

  else if (podatak.gas < 1500) {

    motor(-1);
  }

  else {

    motor(0);
  }


  // ==================================================
  // SERIAL MONITOR
  // ==================================================

  Serial.print("Gas: ");
  Serial.print(podatak.gas);

  Serial.print("  Volan: ");
  Serial.print(podatak.volan);

  Serial.print("  Ciljani kut: ");
  Serial.println(ciljaniKut);
}


// ==================================================
// SETUP
// ==================================================

void setup() {

  Serial.begin(115200);


  // ==================================================
  // WIFI
  // ==================================================

  WiFi.mode(WIFI_STA);


  // ==================================================
  // MOTOR
  // ==================================================

  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);


  // ==================================================
  // ŽMIGAVCI
  // ==================================================

  pinMode(lijeviZmigavac, OUTPUT);
  pinMode(desniZmigavac, OUTPUT);

  ugasiZmigavce();


  // ==================================================
  // SERVO
  // ==================================================

  volan.attach(16);

  volan.write(90);

  trenutniKut = 90;
  ciljaniKut = 90;


  // ==================================================
  // ESP-NOW
  // ==================================================

  if (esp_now_init() != ESP_OK) {

    Serial.println("ESP-NOW greska");

    return;
  }


  esp_now_register_recv_cb(primiPodatak);


  Serial.println("Auto spreman");
}


// ==================================================
// LOOP
// ==================================================

void loop() {


  // ==================================================
  // GLATKO POMICANJE SERVA
  // ==================================================

  if (trenutniKut < ciljaniKut) {

    trenutniKut += 3;

    if (trenutniKut > ciljaniKut) {

      trenutniKut = ciljaniKut;
    }

    volan.write(trenutniKut);
  }


  else if (trenutniKut > ciljaniKut) {

    trenutniKut -= 3;

    if (trenutniKut < ciljaniKut) {

      trenutniKut = ciljaniKut;
    }

    volan.write(trenutniKut);
  }


  // ==================================================
  // ŽMIGAVCI
  // ==================================================

  // RIKVERC
  // OBA ŽMIGAVCA

  if (podatak.gas < 1500) {

    treperiZmigavac(1, 1);
  }


  // NIJE RIKVERC
  // GLEDAMO VOLAN

  else {

    // LIJEVO

    if (podatak.volan < 1765) {

      treperiZmigavac(1, 0);
    }


    // DESNO

    else if (podatak.volan > 1965) {

      treperiZmigavac(0, 1);
    }


    // SREDINA

    else {

      ugasiZmigavce();

      stanjeTrepere = false;
      zadnjeTrepere = millis();
    }
  }


  delay(10);
}