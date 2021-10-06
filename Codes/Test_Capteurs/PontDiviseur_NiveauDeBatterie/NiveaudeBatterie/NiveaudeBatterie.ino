/*
pour 

4.7 to 3.3V    ┌─────┐         A0 (arduino)
    ──────┤   R1    ├────┬───►
               └─────┘       |
                              ┌─┴─┐
                              │     │
                              │R2   │
                              │     │
                              └─┬─┘
                                  |
                                  |
                                  |
                                 ▼

Avec R1 = 180 KΩ
Avec R2 = 220 KΩ
*/

int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

}

void loop() {
  int i = 0;
  int cap = 0;
  // on fait la moyenne sur 10 valeurs
  while (++i <= 10)
  {
    Serial.print(i);
    Serial.print("\t- sensor = ");
    Serial.println(sensorValue);
    cap += sensorValue;
    delay(200);
  }
  cap /= 10;
   Serial.print("sensor = ");
    Serial.println(cap);
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  delay(2000);
}
