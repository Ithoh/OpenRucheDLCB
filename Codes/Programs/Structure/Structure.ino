/*
 * Structure pour les programmes du projet 
 * (declarations et initialisation des capteurs + fonction de lectures)
 */
  #define ADC_resolution 10
  
  float rap_V_ADC = 3.2;  //rapport for avoir la tension en mV.
  int bat_brut;
  int pan_brut;
  float bat_tension;
// Declaration for lecture de tension
  #define batterie_in A0
  #define panneau_solaire_in A3
  #define rap_pan 0.6   // R2/R1+R2
  #define rap_bat 0.785 // R2/R1+R2
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
 * for le capteur de la panneau
 * Avec R1 = 180 KΩ
 * Avec R2 = 220 KΩ 
 * for le capteur du batterie
 * Avec R1 = XX KΩ
 * Avec R2 = XX KΩ
 */


 //Declaratoin for DS18B20
  #include <OneWire.h>
  #include <DallasTemperature.h>
  
  OneWire ds18x20[] = {8, 9, 10};
  const int oneWireCount = sizeof(ds18x20)/sizeof(OneWire);
  DallasTemperature sensor[oneWireCount];

//Declaration for DHT
  #include "DHT.h"
  #define DHTPIN 7
  #define DHTTYPE DHT22 
  DHT dht(DHTPIN, DHTTYPE);

//Declaration for Sigfox
  #include <SigFox.h>
  typedef struct __attribute__((packed)) sigfox_message
  {
    int8_t temp_dht;
    int8_t humidity;
    int8_t temp_ext_dht;
    int8_t humidity_ext;
    int16_t weight;
    int8_t temp_DS1;
    int8_t temp_DS2;
    int8_t temp_DS3;
    int8_t charge;  
  } SigfoxMessage;
  
  SigfoxMessage msg;
  
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
  
  float calibration_factor = 21710;
  long zero_factor = 36242;
  

//SETUP
void setup() 
{
  //Serial Setup
    Serial.begin(9600);
  
  //Sigfox Setup
    if (!SigFox.begin())
    {
      Serial.println("SigFox error, rebooting");
      reboot();
    }
    delay(100); 
    SigFox.debug();
    SigFox.status();
    delay(1); 

  //Voltage reading Setup
    pinMode(panneau_solaire_in, INPUT);
    pinMode(batterie_in, INPUT);

  //HX711 Setup 
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(calibration_factor);
    scale.tare(zero_factor); //Reset the scale to 0

  //DHT Setup
    dht.begin();
    delay(100); 

  //DS18B20 Setup
    DeviceAddress deviceAddress;
    for (int i = 0; i < oneWireCount; i++) 
    {
      sensor[i].setOneWire(&ds18x20[i]);
      sensor[i].begin();
      if (sensor[i].getAddress(deviceAddress, 0))
      { 
        sensor[i].setResolution(deviceAddress, 8);
      }
    }
}

//INFINITE LOOP
void loop() 
{
  //CODE HERE
}

void read_ds18B20()
{
  for (int i = 0; i < oneWireCount; i++) {
    sensor[i].requestTemperatures();
  }
  delay(1000);
  msg.temp_DS1 = (int)(sensor[0].getTempCByIndex(0) * 2);
  msg.temp_DS2 = (int)(sensor[1].getTempCByIndex(0) * 2);
  msg.temp_DS3 = (int)(sensor[2].getTempCByIndex(0) * 2);
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
  msg.temp_dht = int8_t(t * 100);
  msg.humidity = int8_t(h * 100);
  // Wait a few seconds between measurements.
  delay(1000);
}


void send_sigfox(SigfoxMessage message)
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
  msg.weight = scale.get_units()*100;
}

void read_tension()
{
  bat_brut=analogRead(batterie_in);
  
  //pan_brut=analogRead(panneau_solaire_in);

  bat_tension=(bat_brut * rap_V_ADC);
  
  //pan_tension=(pan_brut * rap_V_ADC);

  bat_reel=(bat_tension / rap_bat);
  
  //pan_reel=(pan_tension / rap_pan);
}
