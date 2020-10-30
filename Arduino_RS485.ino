/*

  Basic.pde - example using ModbusMaster library

  Library:: ModbusMaster
  Author:: Doc Walker <4-20ma@wvfans.net>

  Copyright:: 2009-2016 Doc Walker

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

#include <ModbusMaster.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned int RPM = 0;
int M = 0;
uint8_t result;
int start = 0;
uint16_t data[6];


#define MAX485_DE  2
#define MAX485_RE  3
#define buzzer 8
#define MOTOR_POLES 6

// instantiate ModbusMaster object
ModbusMaster node;

void preTransmission()
{
  digitalWrite(MAX485_RE, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);
}

void setup()
{
  pinMode(buzzer, OUTPUT);
  pinMode(MAX485_RE, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Initialize at receive mode
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);

  //LCD_I2C Initialization
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("Welcome Aries!");
  delay(3000);
  lcd.clear();

  // Modbus communication runs at 9600 baud
  Serial.begin(9600); // default: no parity
  Serial2.begin(9600,SERIAL_8E1); //even parity

  // Modbus slave ID 1 
  node.begin(0x01, Serial2);

  
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

void loop()
{
  
  if(Serial.available()){
    String Input = Serial.readString();
    if(Input.equals("Initial")){
      lcd.setCursor(2,1);
      lcd.print("Initializing");
      node.writeSingleRegister(0x006a, 730); // 額定最大電流配置，效率取70%，實際數值*100
      tone(buzzer, 523,1000);
      delay(1000);
      node.writeSingleRegister(0x006b, 730); // 最大負載電流配置，無特別要求則與額定電流相同
      tone(buzzer, 659,1000);
      delay(1000);
      node.writeSingleRegister(0x0073, MOTOR_POLES); // 配置極個數
      tone(buzzer, 784, 1000);
      delay(1000);
      node.writeSingleRegister(0x0074, 1); // 配置減速比
      tone(buzzer, 932, 1000);
      delay(1000);
      node.writeSingleRegister(0x0070, 0); // 配置速度閉環控制算法
      tone(buzzer, 1046, 1000);
      delay(1000);
    }
    else if(Input.equals("Learning")){
      lcd.setCursor(1,1);
      lcd.print("Machine Learning");
      node.writeSingleRegister(0x00e1, 1); // 機器學習
    }
    else if(Input.equals("RPM")){
      if(start == 0){
        start = 1;
      }
      else{
        start =0;
      }
    }
    else if(Input.equals("Control")){
      int RevolutionPerMinute = 10000;
      node.writeSingleRegister(0x0043, (RevolutionPerMinute/60 * MOTOR_POLES *2 *10)); // RPM/60*極個數*2 *10(0x0043倍率) = 電機換相頻率
    }
    else if(Input.equals("Stop")){
      int RevolutionPerMinute = 0;
      node.writeSingleRegister(0x0043, 0);
    }
  }

  if(start == 1){
    // Read 2 registers starting at 0x0034)
    result = node.readHoldingRegisters(0x0034, 2);
    if (result == node.ku8MBSuccess){
      RPM = node.getResponseBuffer(0x00);
      M = node.getResponseBuffer(0x01);

      Serial.print("RPM:");
      Serial.print(RPM/20*3);
      Serial.print("M:");
      Serial.println(M);
      if(M == 1){
        lcd.setCursor(5,1);
        lcd.print((RPM*10));
        //Serial.print("RevoStoplutionPerMinute:  "); 
        //Serial.println((RPM*10));
      }
      else if(M ==0){
        lcd.setCursor(5,1);
        lcd.print(RPM);
        //Serial.print("RevolutionPerMinute:  "); 
        //Serial.println(RPM);
      }
    }
    delay(1000);
    lcd.setCursor(9,1);
    lcd.clear();
  }

  lcd.setCursor(2,0);
  lcd.print("Spindle RPM:");
}
