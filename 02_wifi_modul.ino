// dolocimo uporabljene knjiznice
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

// nastavitve za wifi povezavo
const char* ssid = "xxx";
const char* password = "yyy";
WiFiClient  client;

// nastavitve za posiljanje podatkov na sistem ThingSpeak
unsigned long myChannelNumber = 00000;
const char * myWriteAPIKey = "ABCD";

void setup() {
  // put your setup code here, to run once:
  // vzpostavimo serijsko povezavo
  Serial.begin(9600);

  // vzpostavi wifi povezavo s prej doloceno dostopno tocko
  WiFi.begin(ssid, password);
  // povezava lahko traja nekaj casa, zato vmes izpisujemo pikice
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Izpisemo pridobljen IP naslov
  Serial.print("Use this URL to connect: ");
  Serial.println(WiFi.localIP());

  // nastavimo povezavo do sistema ThingSpeak
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  // put your main code here, to run repeatedly:
  // pogledamo ali imamo kaj podatkov na voljo za branje na serijskem vmesniku
  if (Serial.available() > 0) 
  {
    // preberemo iz izpisemo prejeto
    String prejeto = String(Serial.readString());
    Serial.println(prejeto);

    // sedaj dolocimo vse vrednosti, ki so nam bile poslane
    int is = 0;
    int ie = 0;
    // nastavitev ThingSpeak polja v kanalu - vrednosti gredo od 1 do 8, kolikor meritev lahko shranimo v en zastonjski kanal
    int kanal = 1;
    // sprehodimo se cez pridobljen string in poisceno lokacije vejic, ki nam medsebojno delijo poslane vrednosti
    while (prejeto.indexOf(",", is) >= 0)
    {
      ie = prejeto.indexOf(",", is);
      float senzor = prejeto.substring(is, ie).toFloat();
      Serial.println(senzor);
      is = ie + 1;

      // doloci vrednost, ki bo zapisana na izbrano polje
      ThingSpeak.setField(kanal, senzor);
      kanal = kanal + 1;
    }

    // zapisi vse vrednosti polj v nas ThingSpeak kanal
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) 
    {
      // izpis če je zapis vrednosti uspesen
      Serial.println("Channel update successful.");
    }
    else 
    {
      // izpis če zapis vrednosti ni bil uspesen
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }

  }
}
