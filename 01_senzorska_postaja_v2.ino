// knjiznice
#include <DHT.h>  // knjiznica za DHT senzor
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <SoftwareSerial.h>
#include <stdio.h>
#define N 23
#define _SS_MAX_RX_BUFF 23

// konstante
#define PIN_SVET A0
#define PIN_ZVOK A6
#define PIN_CO2 A7
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

// Spremenljivke
int prikaz = 0;
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
unsigned long svetloba;
unsigned long jakost1;
unsigned long co2_volt;
unsigned long t_zaslon, t_poslji;
int poz_napis, poz_vredn, poz_enota;

unsigned char buffer [N];
int PM25 = 0, PM10 = 0, PM1 = 0;
char line1[16], line2[16];

SoftwareSerial PMSerial(A2, A1); // RX, TX

bool checkValue(unsigned char *buf, int length)
{  
  bool flag=0;
  int sum=0;
 
  for(int i=0; i<(length-2); i++)
  {
    sum+=buf[i];
  }
  sum=sum + 0x42;
  
  if(sum == ((buf[length-2]<<8)+buf[length-1]))  
  {
    sum = 0;
    flag = 1;
  }
  return flag;
}

void setup()
{ 
  // inicializacija zaslona
  lcd.init();
  lcd.backlight();
  // inicializacija senzorja za temperaturo in vlago
  dht.begin();
  // nastavimo na kater port smo prikljucili senzor za svetlost
  pinMode(PIN_SVET, INPUT);
  // nastavimo na kater port smo prikljucili zvočnik
  pinMode(PIN_ZVOK, INPUT);
  // nastavimo na kater port smo prikljucili CO2 senzor
  pinMode(PIN_CO2, INPUT);
  // vzpostavimo serijsko povezavo z racunalnikom
  Serial.begin(9600);
  PMSerial.begin(9600);

  // zacni stevce za oddajo podatkov in spremembo prikaza zaslona
  t_zaslon = millis(); 
  t_poslji = millis();
}

float read_analog_value(int in_pin, int read_rep){
  unsigned long a_vrednost = 0;
  for(int ir=0; ir < read_rep; ir++)
  {
    a_vrednost += analogRead(in_pin);
  }
  return 1. * a_vrednost / read_rep;
}

void loop() {
  // put your main code here, to run repeatedly:

  //Read data and store it to variables hum and temp
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  
  // nastavimo zacetne vrednosti najvisje in najnizje mozne vrednosti za prebrano glasnost mikrofona
  int max_jakost1 = 0;
  int min_jakost1 = 1023;

  // preberi napetost mikrofona 1000-krat in doloci najvecjo in najmanjso vrednost od vseh prebranih
  int read_rep = 1000;
  for (int i = 0; i < read_rep; i++) {
    jakost1 = analogRead(PIN_ZVOK);
    max_jakost1 = max(max_jakost1, jakost1);
    min_jakost1 = min(min_jakost1, jakost1);
  }
  int jakost_razpon = max_jakost1 - min_jakost1;
  // izpise spodnjo, zgornjo in razliko napetosti iz mikrofona
  // Serial.println(jakost_razpon);

  // preberemo napetost senzorja za svetlost
  float svetloba_f = read_analog_value(PIN_SVET, 100);

  // preberemo napetost senzorja za co2 koncentracijo
  float co2_volt_f = read_analog_value(PIN_CO2, 1000);

  // preberemo koncentracije razlicno velikih delcev v zraku
  char fel = 0x42;
  if(PMSerial.find(&fel, 1)) {
    PMSerial.readBytes(buffer,N);
  }  
  if(buffer[0] == 0x4d)
  {
    if(checkValue(buffer,N))
    {
        PM25=((buffer[5]<<8) + buffer[6]);
        PM10=((buffer[7]<<8) + buffer[8]);
        PM1=((buffer[3]<<8) + buffer[4]);
    }
  }

  // na vsakih 20 sekund poslji zbrane podatke naprej
  // vsaj 15 sekund zamika zaradi omejitve pisanja prebranih podatkov v bazo
  if(millis() - t_poslji > 20000)
  {
    // poslji podatke po serijski povezavi na drug vmesnik
    Serial.print(String(jakost_razpon) + "," + String(svetloba_f) + "," + String(hum) + "," + String(temp) + "," + String(co2_volt_f) + "," + String(PM1) + "," + String(PM25) + "," + String(PM10) + ",");
    t_poslji = millis();
  }

  // na vsakih 5 sekund spremeni prikaz na zaslonu
  if(millis() - t_zaslon > 5000)
  {
    // pobrisimo kar je zapisano na zaslonu
    lcd.clear();

    poz_napis = 0;
    poz_vredn = 10;
    poz_enota = 14;

    // prikazi pravilen izpis na zaslon
    switch(prikaz)
    {
      // izpisi na LCD zaslon izbran izpis
      case 0:
        lcd.setCursor(poz_napis, 0);
        lcd.print("Temper:");
        lcd.setCursor(poz_vredn, 0);
        lcd.print(temp);
        lcd.setCursor(poz_enota, 0);
        lcd.print("C");
        lcd.setCursor(poz_napis, 1);
        lcd.print("Vlaznost:");
        lcd.setCursor(poz_vredn, 1);
        lcd.print(hum);
        lcd.setCursor(poz_enota, 1);
        lcd.print("%");
        break;
      case 1:
        lcd.setCursor(poz_napis, 0);
        lcd.print("Vlaznost:");
        lcd.setCursor(poz_vredn, 0);
        lcd.print(hum);
        lcd.setCursor(poz_enota, 0);
        lcd.print("%");
        lcd.setCursor(poz_napis, 1);
        lcd.print("Svetlost:");
        lcd.setCursor(poz_vredn, 1);
        lcd.print(svetloba_f);
        break;
      case 2:
        lcd.setCursor(poz_napis, 0);
        lcd.print("Svetlost:");
        lcd.setCursor(poz_vredn, 0);
        lcd.print(svetloba_f);
        lcd.setCursor(poz_napis, 1);
        lcd.print("Glasnost:");
        lcd.setCursor(poz_vredn, 1);
        lcd.print(jakost_razpon);
        break;
      case 3:
        lcd.setCursor(poz_napis, 0);
        lcd.print("Glasnost:");
        lcd.setCursor(poz_vredn, 0);
        lcd.print(jakost_razpon);
        lcd.setCursor(poz_napis, 1);
        lcd.print("Delci 2.5:");
        lcd.setCursor(poz_vredn, 1);
        lcd.print(PM25);
        break;
      case 4:
        lcd.setCursor(poz_napis, 0);
        lcd.print("Delci 2.5:");
        lcd.setCursor(poz_vredn, 0);
        lcd.print(PM25);
        lcd.setCursor(poz_napis, 1);
        lcd.print("Temper:");
        lcd.setCursor(poz_vredn, 1);
        lcd.print(temp);
        lcd.setCursor(poz_enota, 1);
        lcd.print("C");
        break;
      case 5:

        break;
      default:
        break;
    }

    // naslednjic prikazi naslednji zaslonski izpis
    prikaz += 1;
    prikaz %= 5;
    t_zaslon = millis();
  }


}
