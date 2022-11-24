#include <Arduino.h>

#include <LiquidCrystal.h>
#include <MQUnifiedsensor.h>

#include <SoftwareSerial.h>
// MQ2 Config
#define Pin (A0)
#define MQPin (A0) // analogue input pin for sensor
#define Board ("Arduino UNO")
#define Type ("MQ-2") // MQ2
#define Voltage_Resolution (5)
#define ADC_Bit_Resolution (10) // For arduino UNO/MEGA/NANO
#define RatioMQ2CleanAir (9.83) // R1 / R0 = 9.83 ppm
// MQ2 Config  END

// MQ7 Config
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define pin A1                // Analog input 0 of your arduino
#define type "MQ-7"           // MQ7
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ7CleanAir 27.5 // RS / R0 = 27.5 ppm
// MQ7 Config end

//MQ 135 Config
#define pin_mq135 A2 //Analog input 0 of your arduino
#define type_mq135 "MQ-135" //MQ135
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  

#define BuzzerPin 8

// variable to store sensor value
float sensorValue; // 0 - 1023
float ppm, ppm_mq7, ppm_mq135;
float threshold = 25.0;

int rs = 12,
    enable = 11, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, enable, d4, d5, d6, d7);

MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
MQUnifiedsensor MQ7(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin_mq135, type_mq135);

void setup()
{

  Serial.begin(115200);
  lcd.begin(16, 2);

  delay(1000);
  Serial.println("Gas sensor warming up");
  delay(10000);
  Serial.println("Gas sensor ready");

  // Set math model to calculate the PPM concentration and the value of constants
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setA(574.25);
  MQ2.setB(-2.222); // Configure the equation to to calculate LPG concentration

  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setA(99.042);
  MQ7.setB(-1.518); // Configure the equation to calculate CO concentration value

  // mq 135
  //Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(102.2); MQ135.setB(-2.473); // Configure the equation to to calculate NH4 concentration



  MQ2.init();
  MQ7.init();
  MQ135.init();

  // Calibrating MQ 2 ***************************************************
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++)
  {
    MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0))
  {
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    while (1)
      ;
  }
  if (calcR0 == 0)
  {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    while (1)
      ;
  }
  /*****************************  MQ 2 CAlibration ********************************************/

  // Calibrating MQ 7 ***************************************************
  calcR0 = 0;
  for (int i = 1; i <= 10; i++)
  {
    MQ7.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ7.calibrate(RatioMQ7CleanAir);
    Serial.print(".");
  }
  MQ7.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0))
  {
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    while (1)
      ;
  }
  if (calcR0 == 0)
  {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    while (1)
      ;
  }
  /*****************************  MQ 7 CAlibration ********************************************/


  // mq 135 calibration
  Serial.print("Calibrating please wait.");
   calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}

  pinMode(2, OUTPUT);
  // digitalWrite(3, LOW);
  pinMode(BuzzerPin, OUTPUT);
}

void loop()
{
  // read the sensor value
  sensorValue = analogRead(MQPin);
  // Serial.print("MQ2 ADC ");
  // Serial.println(sensorValue);

  MQ2.update();           // Update data, the arduino will read the voltage from the analog pin
  ppm = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  // Serial.print("MQ2 ppm ");
  // Serial.println(String(ppm).c_str());

  lcd.setCursor(0, 0);
  lcd.print(String(ppm).c_str());

  // read the sensor value
  sensorValue = analogRead(pin); // MQ 7 sensor value
  // Serial.print("MQ7 ADC ");
  // Serial.println(sensorValue);

  MQ7.update();               // Update data, the arduino will read the voltage from the analog pin
  ppm_mq7 = MQ7.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  // Serial.print("MQ7 ppm ");
  // Serial.println(String(ppm_mq7).c_str());

  lcd.setCursor(8,0);
  lcd.print(String(ppm_mq7).c_str());

  // read the sensor value
  sensorValue = analogRead(pin_mq135); // MQ 7 sensor value
  // Serial.print("MQ135 ADC ");
  // Serial.println(sensorValue);

  MQ135.update();               // Update data, the arduino will read the voltage from the analog pin
  ppm_mq135 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  // Serial.print("MQ135 ppm ");
  // Serial.println(String(ppm_mq135).c_str());

  lcd.setCursor(0,1);
  lcd.print(String(ppm_mq135).c_str());




   String sendString = "$";
 sendString += String(ppm).c_str();
 sendString += ",";
 sendString += String(ppm_mq7).c_str();
 sendString += ",";
 sendString += String(ppm_mq135).c_str();
 sendString += ",\n";
 Serial.println(sendString.c_str());

  if (ppm > 10.0)
  {
    digitalWrite(2, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.print("warning");
    delay(10000);
    digitalWrite(2, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(BuzzerPin, LOW);
  }

  // delay between readings
  delay(2000);
  lcd.clear();
}