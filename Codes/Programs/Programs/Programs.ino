/*
 * Code de lecture des tension analogiques de la batterie et du panneau solaire
 */
// Declaration for lecture de tension
#define Batterie_in A0
#define Panneau_solaire_in A3
#define rap_pan 0.6   // R1/R1+R2
#define rap_bat 0.785 // R1/R1+R2
#define ADC_resolution 10
float rap_V_ADC;  //rapport for avoir la tension en mV.

int bat_brut;
int pan_brut;

float bat_tension;
float pan_tension;

float bat_reel;
float pan_reel;

//La tension d'ensoleillement maximum a été définie sur 5.5v (incluant une marge de sécurité)
/*
 *              R1
 *           ________
 * Vin------|        |---- vout
 *          |        |      |
 *           ¯¯¯¯¯¯¯¯       |
 *                          |
 *                        ____
 *                       |    |
 *                       |    |
 *                       |    | R2
 *                       |    |
 *                        ¯¯¯¯
 *                          |
 *                          |
 *                          |
 *                        _____
 *                         ---
 *                          ¯
 * for le capteur de la batterie
 * Avec R1 = 180 KΩ
 * Avec R2 = 220 KΩ 
 * for le capteur du panneau
 * Avec R1 = XX KΩ
 * Avec R2 = XX KΩ
 */


 //Declaratoin for DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire ds18x20[] = { 3, 7 };
const int oneWireCount = sizeof(ds18x20)/sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

//Declaration for DHT
#include "DHT.h"
#define DHTPIN 6
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

//Declaration for Sigfox
typedef struct __attribute__((packed)) sigfox_message
{
  int16_t temp;
  int16_t humidity;
  
} SigfoxMessage;

SigfoxMessage msg;

// =================== UTILITIES ===================
void reboot()
{
  NVIC_SystemReset();
  while (1)
    ;
}

//Declaration for HX711
#include "HX711.h"
#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN  2

HX711 scale;

float calibration_factor = -21710;
long zero_factor = -36242
float weight;

void setup() 
{
  Serial.begin(9600);
  while (!Serial)
  ;
  
  //Setup de Sigfox
  if (!SigFox.begin())
  {
    Serial.println("SigFox error, rebooting");
    reboot();
  }
  delay(100); 
  SigFox.debug();
  SigFox.status();
  delay(1); 

  //Setup de la lecture de tension
  pinMode(Panneau_solaire_in, INPUT);
  analogReadResolution(Panneau_solaire_in);
  pinMode(Batterie_in, INPUT);
  analogReadResolution(Batterie_in);
  rap_V_ADC = 2;
  for (int i = 0; i<ADC_resolution;i++)
  {
    rap_V_ADC *= 2;
  }
  rap_V_ADC = 3300/rap_V_ADC;

  //HX711 Setup 
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare(zero_factor); //Reset the scale to 0
}

void loop() 
{
  
}

void read_dht()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  msg.temp = int16_t(t * 100);
  msg.humidity = int16_t(h * 100);
  envoi(msg);

  // Wait a few seconds between measurements.
  delay(1000);
}


void send(SigfoxMessage message)
{
  if (!SigFox.begin())
  {
    Serial.println("SigFox error, rebooting");
    reboot();
  }
  SigFox.status();
  delay(1);
  SigFox.beginPacket();
  SigFox.write((uint8_t *)&message, sizeof(SigfoxMessage));
  Serial.print("Status : ");
  Serial.println(SigFox.endPacket());
  SigFox.end();
}

void read_weight()
{
  weight = scale.get_units();
}

void read_tension()
{
  bat_brut=analogRead(batterie_in);
  Serial.print("Valeurs brut de ADC for la batterie :");
  Serial.println(bat_brut);
  
  pan_brut=analogRead(panneau_solaire_in);
  Serial.print("Valeurs brut de ADC for le panneau :");
  Serial.println(pan_brut);

  bat_tension=(bat_brut*rap_V_ADC);
  Serial.print("Valeurs de tension lu par l'ADC for le panneau :");
  Serial.println(pan_tension);
  
  pan_tension=(pan_brut*rap_V_ADC);
  Serial.print("Valeurs de tension lu par l'ADC for le panneau :");
  Serial.println(pan_tension);

  bat_reel=(bat_tension / rap_bat);
  Serial.print("Valeurs de tension batterie avant pont-diviseur:");
  Serial.println(pan_tension);
  
  pan_reel=(pan_tension / rap_pan);
  Serial.print("Valeurs de tension panneau avant pont-diviseur:");
  Serial.println(pan_tension);
}
