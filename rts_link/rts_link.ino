#include "SomfyRTS.h"
#include <EEPROM.h>

// Serial protocol (one command per line, terminated by '\n'):
//   PROG            -> allocates a new virtual remote, sends PROG, answers "<id>"
//   MULTIPROG;<id>  -> sends PROG with an existing remote, answers "<id>"
//   UP;<id> / DOWN;<id> / STOP;<id> -> answers "OK"
//   PING            -> answers "OK"
//   RESET           -> factory reset: clears ALL remotes AND their rolling codes,
//                      every paired cover must be paired again. Answers "OK"
// Any invalid command answers "ERROR".

// Uncomment to test the serial protocol without emitting anything on the radio
// #define DRY_RUN

SomfyRTS myRTS(3, TSR_RFM69); //Tx pin number, transmitter type
                              //pin number : pin connected to the transmitter DATA pin or to the DIO2 pin on RFM69
                              //transmitter type can be TSR_RFM69 or TSR_AM (for a generic AM 433.42MHZ transmitter)

// The next free remote id is stored in the last EEPROM byte.
// Rolling codes use 2 bytes per remote from address 0, so ids must stay below MAX_COVERS.
const int ID_ADDRESS = EEPROM.length() - 1;
const int MAX_COVERS = 255;
int idCover = 0;

// Somfy motors need a silence between two transmissions, otherwise
// back-to-back commands (grouped actions in HA) are ignored.
const unsigned long MIN_GAP_MS = 500;
unsigned long lastEmission = 0;

void sendRadio(int id, byte cmd) {
#ifndef DRY_RUN
  unsigned long elapsed = millis() - lastEmission;
  if (lastEmission != 0 && elapsed < MIN_GAP_MS) delay(MIN_GAP_MS - elapsed);
  myRTS.sendSomfy(id, cmd);
  lastEmission = millis();
#endif
}

// Parses the id after ';'. Returns -1 if missing, not a number, or not allocated yet.
int parseId(const String &value) {
  int sep = value.indexOf(';');
  if (sep == -1) return -1;
  String idStr = value.substring(sep + 1);
  if (idStr.length() == 0) return -1;
  for (unsigned int i = 0; i < idStr.length(); i++) {
    if (!isDigit(idStr[i])) return -1;
  }
  int id = idStr.toInt();
  if (id < 0 || id >= idCover) return -1;
  return id;
}

void reply(const String &msg) {
  Serial.print(msg);
  Serial.print('\n');
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100); // only used if a line arrives without '\n'

  bool radioOk = myRTS.initRadio();
  myRTS.setHighPower(true); //have to call it after initialize for RFM69HW

  idCover = EEPROM.read(ID_ADDRESS);
  if (idCover >= MAX_COVERS) idCover = 0; // erased/corrupted EEPROM reads 0xFF

  reply("Somfy RTS link");
  if (!radioOk) reply("RADIO ERROR"); // RFM69 not answering: check wiring/power
}

void loop() {
  if (!Serial.available()) return;

  String value = Serial.readStringUntil('\n');
  value.trim();
  if (value.length() == 0) return;

  String cmdName = value;
  int sep = value.indexOf(';');
  if (sep != -1) cmdName = value.substring(0, sep);

  if (cmdName == "PROG") { // Add a new cover
    if (idCover >= MAX_COVERS) {
      reply("ERROR");
      return;
    }
    int id = idCover;
    sendRadio(id, PROG);
    idCover++;
    EEPROM.update(ID_ADDRESS, idCover);
    reply(String(id));
  }
  else if (cmdName == "MULTIPROG") { // Pair one more cover with an existing remote
    int id = parseId(value);
    if (id == -1) {
      reply("ERROR");
      return;
    }
    sendRadio(id, PROG);
    reply(String(id));
  }
  else if (cmdName == "UP" || cmdName == "DOWN" || cmdName == "STOP") {
    int id = parseId(value);
    if (id == -1) {
      reply("ERROR");
      return;
    }
    byte cmd = STOP;
    if (cmdName == "UP") cmd = UP;
    else if (cmdName == "DOWN") cmd = DOWN;
    sendRadio(id, cmd);
    reply("OK");
  }
  else if (cmdName == "PING") {
    reply("OK");
  }
  else if (cmdName == "RESET") { // Factory reset, see header
    for (unsigned int i = 0; i < EEPROM.length(); i++) {
      EEPROM.update(i, 0);
    }
    idCover = 0;
    reply("OK");
  }
  else {
    reply("ERROR");
  }
}
