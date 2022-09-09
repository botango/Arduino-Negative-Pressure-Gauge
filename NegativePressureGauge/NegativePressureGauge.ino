#include <Wire.h>

#define L_PIN               13
#define BTN_PIN             2
#define MIS2500_015V_PIN    A0

#define LCD_ADRS        0x3E
#define LCD_LOCATE(x,y) LCD_command(0x80 | (0x40 * y + x))
#define LCD_CLS()       LCD_command(0x01)

int Lchika_count;
uint16_t offset=0;

void setup()
{
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(L_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("Negative Pressure Gauge");

  analogRead(MIS2500_015V_PIN);

  Wire.begin();
  Wire.setClock(100000);
  LCD_init();
  LCD_CLS();
  
  digitalWrite(L_PIN, HIGH);
  LCD_write("Negative");
  LCD_LOCATE(0,1);
  LCD_write("Pressure");
  delay(1000);
  digitalWrite(L_PIN, LOW);
  LCD_CLS();
  delay(100);
  
  offset = analogRead(MIS2500_015V_PIN);
  Serial.print("offset:");
  Serial.print(offset);
  Serial.println();

}

void loop()
{
  int16_t sensorValue = analogRead(MIS2500_015V_PIN);
  Serial.print("sensorValue:");
  Serial.print(sensorValue);
  Serial.print(" offset:");
  Serial.print(offset);
  
  sensorValue-=offset;
  bool fPositive = false;
  if(sensorValue<0)
  {
    fPositive=true;
    sensorValue=abs(sensorValue);
  }
  
  float _pressure=(sensorValue)/4.5*1000*5/1024;
  int16_t pressure = _pressure;
  
  Serial.print(" pressure:");
  Serial.print(_pressure);
  Serial.print("hPa ");
  Serial.println();
  
  LCD_LOCATE(1,0);
  if(fPositive || pressure==0)    LCD_write(' ');
  else                            LCD_write('-');
  lcd_print_MPa(pressure);
  
  LCD_LOCATE(5,1);
  LCD_write("MPa");

  uint8_t i;
  for(i=0;i<99;i++)
  {
    if (digitalRead(BTN_PIN) == LOW)
    {
      delay(10);
      if (digitalRead(BTN_PIN) == LOW)
      {
        LCD_CLS();
        LCD_LOCATE(0,0);
        LCD_write("Offset");

        offset = analogRead(MIS2500_015V_PIN);
        
        LCD_LOCATE(3,1);
        lcd_print_uint(offset);

        while(digitalRead(BTN_PIN) == LOW);
        LCD_CLS();
        break;
      }      
    }
    delay(10);
  }
  digitalWrite(L_PIN, Lchika_count++&1);
}

void LCD_init()
{
// 3Vで動かす場合
//#define LCD_CONTRAST          (35)
//#define LCD_CONTRAST_H        (0x5c)

// 5Vで動かす場合
#define LCD_CONTRAST        (35)
#define LCD_CONTRAST_H      (0x58)

  delay(50);
  LCD_command(0b00111000);  // ファンクション・セット
  LCD_command(0b00111001);  // ファンクション・セット
  LCD_command(0x14);      // カーソル移動・表示シフト
  
  // Contrast set
  LCD_command(0x70 | (LCD_CONTRAST & 0xF));
  LCD_command(LCD_CONTRAST_H | ((LCD_CONTRAST >> 4) & 0x3));

  LCD_command(0b01101100);  // follower control
  delay(300);

  LCD_command(0b00111000);  // ファンクション・セット
  LCD_command(0b00001100);  // ディスプレイＯＮ

  LCD_command(0x01);      // LCD clear
  delay(2);
}

void LCD_command(uint8_t data)
{
  Wire.beginTransmission(LCD_ADRS);
  Wire.write(0x00);
  Wire.write(data);
  Wire.endTransmission();
  delay(10);
}

void LCD_write(const char data)
{
  Wire.beginTransmission(LCD_ADRS);
  Wire.write(0x40);
  Wire.write(data);
  Wire.endTransmission();
  delay(1);
}

void LCD_write(const char *t_data)
{
  Wire.beginTransmission(LCD_ADRS);
  Wire.write(0x40);
  while (*t_data != NULL)  Wire.write(*t_data++);
  Wire.endTransmission();
  delay(1);
}

void lcd_print_uint(uint16_t val)
{
  LCD_write('0'+val/10000);
  val%=10000;
  LCD_write('0'+val/1000);
  val%=1000;
  LCD_write('0'+val/100);
  val%=100;
  LCD_write('0'+val/10);
  val%=10;
  LCD_write('0'+val);
}

void lcd_print_MPa(uint16_t val)
{
  LCD_write('0'+val/10000);
  val%=10000;
  LCD_write('.');
  LCD_write('0'+val/1000);
  val%=1000;
  LCD_write('0'+val/100);
  val%=100;
  LCD_write('0'+val/10);
  val%=10;
  LCD_write('0'+val);
}
