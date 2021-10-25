/*
 * Structure pour les programmes du projet 
 * (declarations et initialisation des capteurs + fonction de lectures)
 */
  #define ADC_resolution 10
  
  static float rap_V_ADC = 3.2;  //rapport for avoir la tension en mV.
  int bat_brut;
  int pan_brut;
  float bat_tension;
// Declaration for lecture de tension
  #define battery_in A0
  #define panneau_solaire_in A3
  #define rap_pan 0.6   // R2/R1+R2
  #define R2 97.7
  #define R1 32.5
  static float rap_bat = R2/(R1+R2); // R2/R1+R2
  float pan_tension;
  float bat_reel;
  float pan_reel;

  typedef struct batStruct
  {
    float voltage;
    int8_t percent;
  }batCharge;
  

  batCharge Battery[12];  

//Solar maximum intensity voltage is defined on 5.5V (including safety offset)
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
 */


 //Declaratoin for DS18B20 sensors
  #include <OneWire.h>
  #include <DallasTemperature.h>
  
  OneWire ds18x20[] = {8, 9, 10};
  const int oneWireCount = sizeof(ds18x20)/sizeof(OneWire);
  DallasTemperature sensor[oneWireCount];

//Declaration for DHT sensors
  #include "DHT.h"
  #define DHTPIN 7
  #define DHTPIN2 5
  #define DHTTYPE DHT22
  #define DHTTYPE2 DHT11 
  DHT dht(DHTPIN, DHTTYPE);
  DHT dht_ext(DHTPIN2, DHTTYPE2);

//Declaration for Sigfox module
  #include <SigFox.h>
  //Default message
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
    int8_t event;  
  } SigfoxMessage;

  //event message
  typedef struct __attribute__((packed)) sigfox_events
  {
    int8_t event_type;
    int16_t event_val;  
  } SigfoxEvent;
  
  SigfoxMessage msg;
  SigfoxEvent event;

  //Rebooting sigfox function
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

  //Calibration and tare factors
  float calibration_factor = 21710;
  long zero_factor = 36257;
  int last_weight;


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
  
    pinMode(battery_in, INPUT);
    Battery[0].percent = 0;
    Battery[0].voltage = 3;
    Battery[1].percent = 5;
    Battery[1].voltage = 3.3;
    Battery[2].percent = 10; 
    Battery[2].voltage = 3.6;
    Battery[3].percent = 20;
    Battery[3].voltage = 3.7;
    Battery[4].percent = 30; 
    Battery[4].voltage = 3.75;
    Battery[5].percent = 40;
    Battery[5].voltage = 3.79;
    Battery[6].percent = 50;
    Battery[6].voltage = 3.83;
    Battery[7].percent = 60;
    Battery[7].voltage = 3.87;
    Battery[8].percent = 70;
    Battery[8].voltage = 3.92;
    Battery[9].percent = 80;
    Battery[9].voltage = 3.97;
    Battery[10].percent = 90;
    Battery[10].voltage = 4.10;
    Battery[11].percent = 100;
    Battery[11].voltage = 4.20;

  //HX711 Setup 
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(calibration_factor);
    scale.set_offset(zero_factor); //Reset the scale to 0

  //DHT Setup
    dht.begin();
    dht_ext.begin();
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
    msg.event = 0;
    read_ds18B20();
    Serial.println(compteur);
    Serial.print("DS1 :");
    Serial.println((float)(msg.temp_DS1/2));
    Serial.print("DS2 :");
    Serial.println((float)(msg.temp_DS2/2));
    Serial.print("DS3 :");
    Serial.println((float)(msg.temp_DS3/2));
    read_dht();
    Serial.print("Temp_dht :");
    Serial.println(msg.temp_dht);
    Serial.print("Humidity :");
    Serial.println(msg.humidity);

    Serial.print("Temp_dht2 :");
    Serial.println(msg.temp_ext_dht);
    Serial.print("Humidity2 :");
    Serial.println(msg.humidity_ext);
    
    read_weight();
    last_weight = msg.weight;
    Serial.print("Poids :");
    Serial.println(msg.weight);
    read_tension();
    
    Serial.print("Tension :");
    Serial.print(msg.charge);
    Serial.println("%");
    if (msg.charge < 10)
    {
      msg.event += 1;//bat low
      Serial.println("Bat low");
    }
    if (msg.weight < 0.25*last_weight)
    {
      msg.event += 2;
      Serial.println("Vol de la ruche");  
    }
    Serial.println(msg.event);
    delay(1000);
    //sendSigfox(msg);
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
  //reading DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  msg.temp_dht = int8_t(t * 2);
  msg.humidity = int8_t(h);
  
  //reading DHT_EXT
  float h2 = dht_ext.readHumidity();
  float t2 = dht_ext.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h2) || isnan(t2))
  {
    Serial.println(F("Failed to read from 2nd DHT sensor!"));
    return;
  }
  msg.temp_ext_dht = int8_t(t2 * 2);
  msg.humidity_ext = int8_t(h2);
  // Wait a few seconds between measurements.
  delay(1000);
}


void sendSigfox(SigfoxMessage message)
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
  bat_brut=analogRead(battery_in);
  bat_tension=(bat_brut * rap_V_ADC);
  bat_reel=(bat_tension / rap_bat);
  for (int i =0 ; i < 12 ; i++)
  {
    if ((bat_reel/1000) < Battery[i].voltage)
    {      
       msg.charge = Battery[i-1].percent;
       break;
    }
  }
}
