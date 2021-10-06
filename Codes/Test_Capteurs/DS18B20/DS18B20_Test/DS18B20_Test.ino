#include "OneWire.h"
#include "DallasTemperature.h"
#define ONE_WIRE_BUS_1 7
OneWire ourWire1(ONE_WIRE_BUS_1);
DallasTemperature sensor1(&ourWire1);

float RawValue =0;


void setup(){
  //Demmarage du capteur
  sensor1.begin();
  sensor1.setResolution(11);
}
void loop(){
  //Lecture des données
  sensor1.requestTemperatures();
  
  //Enregsitrement en degré celsius
  float RawValue = sensor1.getTempCByIndex(0);
  
  //Affichage
  Serial.print("Temp :");
  Serial.println(RawValue);
  delay(1000);
}
