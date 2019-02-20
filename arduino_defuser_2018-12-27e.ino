#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>  // for NRF24 radio library RF24
#include "RF24.h"

LiquidCrystal_I2C lcd(0x27,2, 1, 0, 4,5,6,7,3,POSITIVE);   // Set the LCD I2C address

const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 5, 6, 7, 8 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 2, 3, 4 }; 

unsigned long time_now, time_previous;

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

#define LEDPIN 13
#define CE_PIN   9
#define CSN_PIN 10

// NOTE: the "LL" at the end of the constant is "LongLong" type
const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

int CursorPosition;
#define MESSAGE_LENGTH 8
char Message[MESSAGE_LENGTH] = {0,0,0,0,0,0,0,0};

void setup()
{
  pinMode(LEDPIN,OUTPUT);
  digitalWrite(LEDPIN, LOW);
  Serial.begin(9600);

  lcd.begin(16,2);               // initialize the lcd 
  lcd.home ();                   // go home
  CursorPosition = 0;
  lcd.print("*=delete #=send");       
  lcd.setCursor (CursorPosition, 1 );        
  lcd.print('_');    

  radio.begin();
  radio.setDataRate(RF24_2MBPS);  
  radio.openWritingPipe(pipe);
  delay(500);
  radio.write("R", MESSAGE_LENGTH);         
}

void loop()
{
  char key = kpd.getKey();
  if(key)  // Check for a valid key.
  {
    switch (key)
    {
      case '*':
        if (CursorPosition > 0)
          {
          lcd.setCursor (CursorPosition, 1 );        
          lcd.print(' ');       
          CursorPosition--;       
          lcd.setCursor (CursorPosition, 1 );        
          lcd.print('_');       
          Message[CursorPosition]=0;
          }
        break;
      case '#':
        digitalWrite(LEDPIN, HIGH);
        delay(10);
        digitalWrite(LEDPIN, LOW);

        radio.write(Message, MESSAGE_LENGTH);

        for (int i=0; i<CursorPosition; i++)
          {
          Serial.println(Message[i]);
          Message[i] = '-';
          }
        lcd.setCursor(0,1);
        lcd.print("_              ");       
        CursorPosition = 0;
        break;

      default:
        if (CursorPosition<MESSAGE_LENGTH)
          {
          Message[CursorPosition] = key;
          lcd.setCursor (CursorPosition, 1 );        
          lcd.print(key);
          CursorPosition++; 
          lcd.setCursor (CursorPosition, 1 );        
          lcd.print('_');       
          radio.write("K", MESSAGE_LENGTH);         
          }
    }   
  }
}
