/*
 * Code de lecture des tension analogiques de la batterie et du panneau solaire
 */

#define batterie_in A0
#define panneau_solaire_in A3
#define rap_pan 0.6   // R1/R1+R2
#define rap_bat 0.785 // R1/R1+R2
#define rap_V_ADC 3.2 //analogRead()*3.2 = tension en mV.

int bat_brut;
int pan_brut;
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
 * pour le capteur de la batterie
 * Avec R1 = 180 KΩ
 * Avec R2 = 220 KΩ 
 * pour le capteur du panneau
 * Avec R1 = XX KΩ
 * Avec R2 = XX KΩ
 */
void setup() 
{
  pinMode(panneau_solaire_in, INPUT);
  pinMode(batterie_in, INPUT);
  Serial.begin(9600);

}

void loop() 
{
  bat_brut=analogRead(batterie_in);
  Serial.print("Valeurs brut de ADC pour la batterie :");
  Serial.println(bat_brut);
  
  pan_brut=analogRead(panneau_solaire_in);
  Serial.print("Valeurs brut de ADC pour le panneau :");
  Serial.println(pan_brut);

  //bat_tension=(bat_brut*rap_V_ADC);
  //Serial.print("Valeurs de tension lu par l'ADC pour le panneau :");
  //Serial.println(pan_tension);
  
  //pan_tension=(pan_brut*rap_V_ADC);
  //Serial.print("Valeurs de tension lu par l'ADC pour le panneau :");
  //Serial.println(pan_tension);
}
