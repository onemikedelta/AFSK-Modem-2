#include "FX429.c"

#define PTT A1
#define LED1 A2   //RED
#define LED2 A3   //Green
#define LED3 A4   //Blue

int preamblecount;
//int postamblecount;


byte tx_buffer[64];
byte tx_data_size = 0;
byte tx_buffer_counter = 0;

byte rx_buffer[128];
byte rx_data_counter = 0;
bool data_received = false;

void modem_ISR()
{
  byte modem_status = 0;
  delayMicroseconds(5);
  modem_status = read_byte(STATUS_REG);

//  if(modem_status == 0)
//     modem_status = read_byte(STATUS_REG);
    
//  Serial.print("sta:");Serial.println(modem_status,HEX);

  if(modem_status & 0x08)                                       //TX Data ready
  {if(preamblecount<PREAMBLE)
      {preamblecount++;
       write_byte(0xAA,TXDATA_REG);                             //transmit preamble
      }
   else if(preamblecount==PREAMBLE)                 //Send SYNC
        { write_byte(0xC4,TXDATA_REG);   
          preamblecount++;
        }   
   else if(preamblecount==PREAMBLE+1)               //Send SYNC
        { write_byte(0xD7,TXDATA_REG);   
          preamblecount++; 
        }   
   else if(preamblecount==PREAMBLE+2)               //Turn on internal checksum
        { write_byte(0x03,CONTROL_REG);   
          preamblecount++;
          tx_buffer_counter = 1; 
          write_byte(tx_buffer[0],TXDATA_REG);
        }   
   
   else if(tx_buffer_counter <= tx_data_size)       
      { 
        write_byte(tx_buffer[tx_buffer_counter++],TXDATA_REG);
      }
      /*
      else if(postamblecount < POSTAMBLE)       
      { write_byte(0x00,TXDATA_REG);
        postamblecount++;
      }
      */
  }
  
  if(modem_status & 0x01)                                       //Rx Data ready
  {   
      rx_buffer[rx_data_counter++] = read_byte(RXDATA_REG);
  }

  if(modem_status & 0x20)                                       //Timer Interrupt
  {
      if( !(modem_status & 0x04) )                              //carrier detect
       {digitalWrite(LED2,LOW);}    //Turn off rx led when carrier lost, depends on timer interrupt
        
      if((modem_status & 0xD9) == 0)
       {write_byte(RXMODE, CONTROL_REG);}   //turn off timer if there is no other interrupt
}
  
  if(modem_status & 0x40)                                           //RX Sync Detect
      {
        digitalWrite(LED2,HIGH);    //Turn on RX Led
        write_byte(TIMER_INT | RXMODE, CONTROL_REG);              //Enable timer for carrier sense
        data_received = true;
//        Serial.print("\nSYNC, ");
      }
  
  else if(modem_status & 0x80)                                       //RX Synt Detect
      {
        digitalWrite(LED2,HIGH);    //Turn on RX Led
        write_byte(TIMER_INT | RXMODE, CONTROL_REG);              //Enable timer for carrier sense
        data_received = true;
//        Serial.print("\nSYNT, ");
      }

   if(modem_status & 0x10)                                       //Tx Idle -> end transmission
      {
        write_byte(0x01,CONTROL_REG);
        //delay(POSTAMBLE);
        digitalWrite(PTT, LOW);
        digitalWrite(LED1, LOW);  
        write_byte(RXMODE, CONTROL_REG);
      }   
/*
    if(modem_status & 0x04)                              //carrier detect
      {
        
      }
*/
}


//Interrupt based data transmission
void modem_txi()
{
  tx_buffer_counter = 0;
  preamblecount = 1;
//  postamblecount = 0;
  digitalWrite(PTT,HIGH);            
  digitalWrite(LED1,HIGH); 
  write_byte(0x01,CONTROL_REG);
  delay(PREAMBLE*10);
  //delay(500);
  write_byte(0xAA,TXDATA_REG);      //write first byte of preamble
}


//Pin pooling method data transmission function, not fully tested
/*
void modem_tx()
{
  detachInterrupt(digitalPinToInterrupt(MODEM_IRQ));
  tx_buffer_counter = 0;
  preamblecount = 0;
  digitalWrite(PTT,HIGH);            
  digitalWrite(LED1,HIGH); 
  write_byte(0x01,CONTROL_REG);
  delay(PREAMBLE*10);
  //delay(500);
  
  while (tx_buffer_counter < tx_data_size)       
      {// Serial.println((char)tx_buffer[tx_buffer_counter]);
        write_byte(tx_buffer[tx_buffer_counter++],TXDATA_REG);
        //while(!(read_byte(STATUS_REG)&0x08));
        while(digitalRead(MODEM_IRQ));
      }
  delay(5);
  write_byte(0x01,CONTROL_REG);
  delay(POSTAMBLE*7);
   
  digitalWrite(PTT,LOW);
  digitalWrite(LED1,LOW);  
  write_byte(RXMODE,CONTROL_REG);
  
  attachInterrupt(digitalPinToInterrupt(MODEM_IRQ),modem_ISR, FALLING);
}
*/

void setup() {
 
  pinMode(PTT,OUTPUT);
  digitalWrite(PTT,LOW);

  Serial.begin(115200);
  Serial.println("FX429  AFSK Modem by TA1MD");

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  
  digitalWrite(LED1,LOW); 
  digitalWrite(LED2,LOW); 
  digitalWrite(LED3,LOW); 
              
  modem_init();
  
}

void loop() {
  
  
delay(1000);
digitalWrite(LED3,!digitalRead(LED3));      //blink led
  
  //if(rx_data_counter!=0)
  if(data_received == true)
  {  for(int i=0;i<rx_data_counter;i++)
      {Serial.print(rx_buffer[i],HEX); Serial.print(",");}
       
    Serial.println("");
    Serial.print(rx_data_counter); Serial.println("  Bytes received.");
    rx_data_counter=0;
    data_received = false;
  }
    

}



void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
  }
}
