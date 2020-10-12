#include "Arduino.h"

/*
#define MODEM_D0 2
#define MODEM_D1 4
#define MODEM_D2 5 
#define MODEM_D3 6
#define MODEM_D4 7
#define MODEM_D5 8
#define MODEM_D6 9
#define MODEM_D7 10
*/
#define  MODEM_A0 11
#define  MODEM_A1 12
#define  MODEM_A2 A0
#define  MODEM_STB 13
#define  MODEM_IRQ 3
const byte datapins[8]={2,4,5,6,7,8,9,10};

#define RXMODE 0x04   //requiring SYNC or SYNT
//#define RXMODE 0x0C    //continuous Rx
//#define TXMODE 0x03
//#define TXMODE 0x01

#define PREAMBLE 10
#define POSTAMBLE 75
#define TIMER_INT 0xF0

#define CONTROL_REG  3
#define STATUS_REG   7
#define RXDATA_REG   6
#define TXDATA_REG   2
#define SYNDROME_LREG 4
#define SYNDROME_HREG 5

#define SYNC_H 0xC4
#define SYNC_L 0xD7
#define SYNT_H 0x3B
#define SYNT_L 0x28

void modem_ISR();

void write_byte(uint8_t data,uint8_t reg)
{
  digitalWrite(MODEM_A2,bitRead(reg,2));
  digitalWrite(MODEM_A1,bitRead(reg,1));
  digitalWrite(MODEM_A0,bitRead(reg,0));

  for(int i=0;i<8;i++)
    {pinMode(datapins[i],OUTPUT);
     digitalWrite(datapins[i],bitRead(data,i));
    }
  delayMicroseconds(3);

  digitalWrite(MODEM_STB,LOW);  
  delayMicroseconds(5);  
  digitalWrite(MODEM_STB,HIGH);  
  delayMicroseconds(3);
}


byte read_byte(byte reg)
{
  byte rdata = 0;
  delayMicroseconds(3);
    
  for(int i=0;i<8;i++)
     pinMode(datapins[i],INPUT_PULLUP);

  digitalWrite(MODEM_A2,bitRead(reg,2));
  digitalWrite(MODEM_A1,bitRead(reg,1));
  digitalWrite(MODEM_A0,bitRead(reg,0));
  delayMicroseconds(3);

  digitalWrite(MODEM_STB,LOW);
  delayMicroseconds(3);

  for(int i=0;i<8;i++)
    if(digitalRead(datapins[i]))
       rdata+=bit(i);

  digitalWrite(MODEM_STB,HIGH);
  delayMicroseconds(3); 
  return rdata;
}


void modem_init()
{
  pinMode(MODEM_IRQ,INPUT_PULLUP);
  pinMode(MODEM_STB,OUTPUT);
  pinMode(MODEM_A0,OUTPUT);
  pinMode(MODEM_A1,OUTPUT);
  pinMode(MODEM_A2,OUTPUT);
  
  digitalWrite(MODEM_STB, HIGH);
  write_byte(RXMODE, CONTROL_REG);
  attachInterrupt(digitalPinToInterrupt(MODEM_IRQ),modem_ISR, FALLING); 
}
