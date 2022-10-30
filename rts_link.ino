#include <SomfyRTS.h>
#include <EEPROM.h>


SomfyRTS myRTS(3, TSR_RFM69); //Tx pin number, transmitter type
                              //pin number : pin connected to the transmitter DATA pin or to the DIO2 pin on RFM69
                              //transmitter type can be TSR_RFM69 or TSR_AM (for a generic AM 433.42MHZ transmitter)
int idCover = 10;
int inc = 0;
void setup() {

  Serial.begin(56700);
  Serial.write("20;00;Somfy RTS link V1.1;");
  myRTS.setHighPower(true); //have to call it after initialize for RFM69HW

  idCover = EEPROM.read(255);
  Serial.write(String("Latest ID : " + String(idCover) + ";").c_str());
}


//20;12;TriState;ID=02a0aa;SWITCH=0;CMD=ON;
//20;02;RTS;ID=669851;SWITCH=01;CMD=DOWN;
//10;RTS;001;0;UP;
//20;01;CMD UNKNOWN;

void loop() {
    if(Serial.available()) { //wait command
        String value = Serial.readString();
        if(value.indexOf("10") == 0 && value.indexOf("rts") > 0) { //if command is RTS
            inc++;
            String zero = "";
            if(inc < 10) zero = "0";
            
            if(value.indexOf("PROG") > 0) { //Add cover
              int id = idCover;
              if(value.indexOf("MULTIPROG") > 0){
                id = value.substring(value.indexOf(";",5)+1,value.indexOf(";",5)+4).toInt(); //Find cover ID
              } 
                Serial.write(String("20;" + zero + String(inc) + ";OK;").c_str());
                myRTS.sendSomfy(id, PROG);
                String str = "20;01;RTS;ID=" + zero + String(id) + ";SWITCH=0;CMD=DOWN;";
                Serial.write(str.c_str());//Send new cover
                
                if(value.indexOf("MULTIPROG") == -1){
                  idCover++;
                  EEPROM.write(255, idCover);     
                }     
            }
            else if(value.indexOf("UP") || value.indexOf("DOWN") || value.indexOf("STOP")) { //Exec CMD
              int id = value.substring(value.indexOf(";",5)+1,value.indexOf(";",5)+4).toInt(); //Find cover ID
              int cmd = 0;

              //Detect Command
              if(value.indexOf("UP") > 0) cmd = UP;
              else if(value.indexOf("DOWN") > 0) cmd = DOWN;
              else cmd = STOP;

              //Send command
              if(myRTS.sendSomfy(id, cmd))
                Serial.write(String("20;" + zero + String(inc) + ";OK;").c_str());
            }
            else { //Send error
              Serial.write(String("20;" + zero + String(inc) + ";CMD UNKNOWN;").c_str());
            }
        }
        else if(value.indexOf("RESET") >= 0) { //Clear EEPROM
          for(int i=0;i<255;i++) {
            EEPROM.write(i,0);
          }
          EEPROM.write(255,10);
          Serial.println("EEPROM RESET");
        }
        else if(value.indexOf("REMOVE") > 0) {
          for(int i=0;i<256;i++) {
            myRTS.sendSomfy(i, PROG);
          }
        }
        else {
          Serial.write(String("20;ERROR").c_str());
        }
    }
}
