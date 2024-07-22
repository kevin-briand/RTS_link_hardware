#include "SomfyRTS.h"
#include <EEPROM.h>


SomfyRTS myRTS(3, TSR_RFM69); //Tx pin number, transmitter type
                              //pin number : pin connected to the transmitter DATA pin or to the DIO2 pin on RFM69
                              //transmitter type can be TSR_RFM69 or TSR_AM (for a generic AM 433.42MHZ transmitter)
int idCover = 0;

void setup() {

  Serial.begin(115200);
  Serial.write("Somfy RTS link\n");
  myRTS.setHighPower(true); //have to call it after initialize for RFM69HW

  idCover = EEPROM.read(EEPROM.length()-1);
}

void loop() {
    if(Serial.available()) { //wait command
        String value = Serial.readString();
            
        if(value.indexOf("PROG") != -1) { //Add cover
          int id = idCover;
          if(value.indexOf("MULTIPROG") != -1) {
            id = value.substring(value.indexOf(";")+1).toInt(); //Find cover ID
          } 
          myRTS.sendSomfy(id, PROG);
          Serial.write(String(String(id) + "\n").c_str()); //Send new ID
                  
          if(value.indexOf("MULTIPROG") == -1) {
            idCover++;
            EEPROM.update(EEPROM.length()-1, idCover);
          }     
        }
        else if(value.indexOf("UP") != -1 || value.indexOf("DOWN") != -1 || value.indexOf("STOP") != -1) { //Exec CMD
          int id = value.substring(value.indexOf(";")+1).toInt(); //Find cover ID
          int cmd = 0;

          //Detect Command
          if(value.indexOf("UP") != -1) cmd = UP;
          else if(value.indexOf("DOWN") != -1) cmd = DOWN;
          else cmd = STOP;

          //Send command
          myRTS.sendSomfy(id, cmd);
          Serial.write(String("OK\n").c_str());
        }
        else if(value.indexOf("RESET") != -1) { //Clear EEPROM
          for(int i=0;i<EEPROM.length();i++) {
            EEPROM.update(i,0);
          }
          Serial.write(String("OK\n").c_str());
        }
        else { //Send error
          Serial.write(String("ERROR\n").c_str());
        }   
    }
}
