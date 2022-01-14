//#define DEBUG
#define DEBUGSIGFOX
  
  #include "ArduinoLowPower.h"
//LED
#define LED_PIN 5
//ADC init
  #define ADC_resolution 10
  static float rap_V_ADC = 3.2;  //rapport for avoir la tension en mV.
  int bat_brut;
  float bat_tension;
  
//Voltage reading init
  //battery reading
  #define battery_in A6
  #define R2 100.4
  #define R1 33.09
  static float rap_bat = R2/(R1+R2);
  float bat_reel;
  typedef struct batStruct
  {
    float voltage;
    int8_t percent;
  }batCharge;
  
  batCharge Battery[12];  
  //Luminosity reading
  #define V_LUM_IN A0
  #define R3 56000

  int lum_in=0;

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


 // Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 10 on the Arduino
#define ONE_WIRE_BUS 14
#define TEMPERATURE_PRECISION 10

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress Thermometer1, Thermometer2, Thermometer3;

//Declaration for DHT sensors

  #include <DHT.h>
  #define DHTPIN 12
  #define DHTPIN2 10 

  #define DHTTYPE1 DHT11
  #define DHTTYPE2 DHT22
  DHT dht(DHTPIN,DHTTYPE1);
  DHT dht_ext(DHTPIN2,DHTTYPE2);



//Declaration for Sigfox module
  #include <SigFox.h>
  
  //message
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
  
  SigfoxMessage msg;

  //Rebooting sigfox function
  void reboot()
  {
    NVIC_SystemReset();
    while (1);
  }

//Declaration for HX711
  #include "HX711.h"
  #define LOADCELL_DOUT_PIN  7
  #define LOADCELL_SCK_PIN  8
  
  HX711 scale;

  //Calibration and tare factors
  float calibration_factor = 21710;
  long zero_factor = 36257;
  int last_weight;


//SETUP
void setup() 
{
  //LED
    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN,HIGH);
    Serial.begin(9600);
    
  //Sigfox Setup
    if (!SigFox.begin())
    {
      reboot();
    }
    delay(100); 
    SigFox.debug();
    SigFox.status();
    delay(1);
     
  //DHT
    dht.begin();
    dht_ext.begin();
    
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
    pinMode(LED_PIN,INPUT);
    pinMode(V_LUM_IN,INPUT);
 //HX711 Setup 
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.set_offset(zero_factor); //Reset the scale to 0
  if (!sensors.getAddress(Thermometer1, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(Thermometer2, 1)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(Thermometer3, 2)) Serial.println("Unable to find address for Device 2");
  
  sensors.setResolution(Thermometer1, TEMPERATURE_PRECISION);
  sensors.setResolution(Thermometer2, TEMPERATURE_PRECISION);
  sensors.setResolution(Thermometer3, TEMPERATURE_PRECISION);
  digitalWrite(LED_PIN,LOW);
}

//INFINITE LOOP
void loop() 
{
  #ifndef DEBUG
  
    //Not Debugging
    digitalWrite(LED_PIN, HIGH);
    
    msg.event = 0;    
    read_ds18B20();    
    read_dht();    
    read_weight();        
    read_voltage();
    
    //Events
    if (msg.charge < 10)
    {
      msg.event += 1;//bat low
    }
    if (msg.weight < 0.25*last_weight)
    {
      msg.event += 2;
    }
    sendSigfox(msg);
    
    #ifdef DEBUG_SIGFOX
      LowPower.sleep(5000);
    #else
      LowPower.sleep(617142);
    #endif
      
    
   #else
    //Debugging
    
    msg.event = 0;
    read_ds18B20();
    Serial.print("DS1 :");
    Serial.println((float)(msg.temp_DS1/2));
    Serial.print("DS2 :");
    Serial.println((float)(msg.temp_DS2/2));
    Serial.print("DS3 :");
    Serial.println((float)(msg.temp_DS3/2));
    
    read_dht();
    //dht22
    Serial.print("Temp_dht :");
    Serial.println(msg.temp_dht);
    Serial.print("Humidity :");
    Serial.println(msg.humidity);
    
    //dht11
    Serial.print("Temp_dht_ext :");
    Serial.println(msg.temp_ext_dht);
    Serial.print("Humidity_ext :");
    Serial.println(msg.humidity_ext);
    
    read_weight();
    last_weight = msg.weight;
    Serial.print("Poids :");
    Serial.println(msg.weight);
    
    read_voltage();
    Serial.print("Tension :");
    Serial.println(bat_reel/1000);
    Serial.print("Charge :");
    Serial.print(msg.charge);
    Serial.println("%");

    //Events
    if (msg.charge < 10)
    {
      msg.event += 1;//bat low
      Serial.println("Bat low");
    }
    if (msg.weight < 0.25 * last_weight)
    {
      msg.event += 2;
      Serial.println("Vol de la ruche");  
    }
    Serial.println(msg.event);
    delay(5000);
   #endif
}

void read_ds18B20()
{
  sensors.requestTemperatures();
  delay(100);
  
  #ifdef DEBUG
    //Affichage des temperatures et adresses
    Serial.print("Adresse DS18B20 1 :");
    printAddress(Thermometer1);
    Serial.print(" temp : ");
    Serial.println(msg.temp_DS1 = (int)(sensors.getTempC(Thermometer1) * 2));
    
    Serial.print("Adresse DS18B20 2 :");
    printAddress(Thermometer2);
    Serial.print(" temp : ");
    Serial.println(msg.temp_DS2 = (int)(sensors.getTempC(Thermometer2) * 2));
    
    Serial.print("Adresse DS18B20 3 :");
    printAddress(Thermometer3);
    Serial.print(" temp : ");
    Serial.println(msg.temp_DS3 = (int)(sensors.getTempC(Thermometer3) * 2));
  #else
  
  //Datas backup only
  msg.temp_DS1 = (int)(sensors.getTempC(Thermometer1) * 2);
  msg.temp_DS2 = (int)(sensors.getTempC(Thermometer2) * 2);
  msg.temp_DS3 = (int)(sensors.getTempC(Thermometer3) * 2);
  #endif
}

void read_dht()
{
  //reading DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
    Serial.println("Error reading DHT11");
  }
  //else
  {
    msg.temp_dht = int8_t(t * 2);
    msg.humidity = int8_t(h);
  }
  delay(1000);
  //reading DHT_EXT
  float h2 = dht_ext.readHumidity();
  float t2 = dht_ext.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h2) || isnan(t2))
  {
    Serial.println("Error reading DHT22");
  }
  //else
  {
    msg.temp_ext_dht = int8_t(t2 * 2);
    msg.humidity_ext = int8_t(h2);
  }
}


void sendSigfox(SigfoxMessage message)
{
  digitalWrite(LED_PIN,HIGH);
  //Sigfox send function
  if (!SigFox.begin())
  {
    reboot();
  }
  SigFox.status();
  delay(1);
  SigFox.beginPacket();
  SigFox.write((uint8_t *)&message, sizeof(SigfoxMessage));
  SigFox.endPacket();
  SigFox.end();
  digitalWrite(LED_PIN,LOW);
}

void read_weight()
{
  scale.power_up();

  //weight reading
  if (scale.wait_ready_timeout(1000)) {
    last_weight = msg.weight;
    msg.weight = scale.get_units()*100;
    Serial.print("HX711 reading: ");
    Serial.println(msg.weight);
  } 
  else {
  Serial.println("HX711 not found.");
  }
  scale.power_down();
}

void read_voltage()
{
  //Voltage reading
  bat_brut=analogRead(battery_in);
  bat_tension=(bat_brut * rap_V_ADC);
  bat_reel=(bat_tension / rap_bat);
  for (int i =0 ; i < 12 ; i++)
  {
    if ((bat_reel/1000.0) < Battery[i].voltage)
    {      
       msg.charge = Battery[i].percent;
       break;
    }
  }
  lum_in = analogRead(V_LUM_IN);
  #ifdef DEBUG
    Serial.print("Lum in :");
    Serial.println(lum_in);
  #endif
}
#ifdef DEBUG
  void printAddress(DeviceAddress deviceAddress)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
    }
  }
 #endif
