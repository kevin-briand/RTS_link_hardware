
#include <SomfyRTS.h>
#include <EEPROM.h>


SomfyRTS myRTS(3, TSR_RFM69); //Tx pin number, transmitter type
                              //pin number : pin connected to the transmitter DATA pin or to the DIO2 pin on RFM69
                              //transmitter type can be TSR_RFM69 or TSR_AM (for a generic AM 433.42MHZ transmitter)
int idCover = 0;
void setup() {

  Serial.begin(56700);
  Serial.println("20;00;Somfy RTS link V1;");
  myRTS.setHighPower(true); //have to call it after initialize for RFM69HW

  idCover = EEPROM.read(255);
}

void loop() {
    if(Serial.available()) { //wait command
        String value = Serial.readString();
        if(value.indexOf("10") == 0 && value.indexOf("rts") > 0) { //if command is RTS
            
            if(value.indexOf("PROG") > 0) { //Add Cover
                Serial.println("20;01;OK;");
                myRTS.sendSomfy(idCover, PROG);
                Serial.println("20;01;RTS;ID=" + String(idCover) + ";SWITCH=0;CMD=DOWN;");//Send new cover
                idCover++;
                EEPROM.write(255, idCover);          
            }
            else if(value.indexOf("UP") || value.indexOf("DOWN") || value.indexOf("STOP")) { //Exec CMD
              int id = value.substring(value.indexOf(";",5)+1,value.indexOf(";",5)+4).toInt(); //Find cover ID
              int cmd = 0;

              //Detect Command
              if(value.indexOf("UP") > 0) cmd = UP;
              else if(value.indexOf("DOWN") > 0) cmd = DOWN;
              else cmd = STOP;

              //Send command
              myRTS.sendSomfy(id, cmd);
              Serial.println("20;01;OK;");
            }
            else { //Send error
              Serial.println("20;01;CMD UNKNOWN;");
            }
        }
        else if(value.indexOf("RESET") == 0) { //Clear EEPROM
              EEPROM.write(255,0);
              Serial.println("EEPROM RESET");
        }
    }
  delay(50);
}
