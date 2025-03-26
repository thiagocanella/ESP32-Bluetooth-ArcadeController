#include <Arduino.h>
#include <BleGamepad.h>

BleGamepad bleGamepad("Arcade 2", "Thiago Canella's Project", 100);


#define latencia 5
#define numOfButtons 11
#define numOfHats 1

byte previousButtonStates[numOfButtons];
byte currentButtonStates[numOfButtons];
byte buttonPins[numOfButtons] = {13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25};
byte physicalButtons[numOfButtons] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

byte previousHatStates[numOfHats * 4];
byte currentHatStates[numOfHats * 4];
byte hatPins[numOfHats * 4] = {26, 27, 32, 33}; // Na ordem UP, LEFT, DOWN, RIGHT.
bool primeiroLoop = true;
bool setDirecionalAnalogico = false;

void setup() {
    setupButtons();
    setupHats();
    setupBleGamepad();
}

void loop() {
  if(primeiroLoop){
    verificaTipoDirecional();
  }


  if (bleGamepad.isConnected()) {
    updateButtonStates();
    updateHatStates();
    sendReportIfChanged();
    delay(latencia);
  }
}

void setupButtons() {
    for (byte i = 0; i < numOfButtons; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        previousButtonStates[i] = HIGH;
        currentButtonStates[i] = HIGH;
    }
}

void setupHats() {
    for (byte i = 0; i < numOfHats * 4; i++) {
        pinMode(hatPins[i], INPUT_PULLUP);
        previousHatStates[i] = HIGH;
        currentHatStates[i] = HIGH;
    }
}

void setupBleGamepad() {
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setAutoReport(false);
    bleGamepadConfig.setWhichAxes(true , true , false, false, false, false, false, false); // Desativar todos os eixos
    bleGamepadConfig.setAxesMin(0x8001);
    bleGamepadConfig.setAxesMax(0x7FFF);
    bleGamepadConfig.setButtonCount(numOfButtons);
    bleGamepadConfig.setHatSwitchCount(numOfHats);
    bleGamepadConfig.setTXPowerLevel(9);

    bleGamepad.begin(&bleGamepadConfig);
}

void updateButtonStates() {
    for (byte i = 0; i < numOfButtons; i++) {
        currentButtonStates[i] = digitalRead(buttonPins[i]);

        if (currentButtonStates[i] != previousButtonStates[i]) {
            if (currentButtonStates[i] == LOW) {
                bleGamepad.press(physicalButtons[i]);
            } else {
                bleGamepad.release(physicalButtons[i]);
            }
        }
    }
}

void updateHatStates() {
    if(setDirecionalAnalogico == false){
      signed char hatValues[numOfHats * 4] = {0};

      for (byte i = 0; i < numOfHats * 4; i++) {
        currentHatStates[i] = digitalRead(hatPins[i]);
      }

      for (byte hatIndex = 0; hatIndex < numOfHats; hatIndex++) {
        hatValues[hatIndex] = getHatValue(hatIndex);
      }

      bleGamepad.setHats(hatValues[0], hatValues[1], hatValues[2], hatValues[3]);
      bleGamepad.setLeftThumb(0,0); 
    } else {
      updateAnalogStates();
    }
}

void updateAnalogStates(){
  int posX = 0;
  int posY = 0;
  signed char hatValues[numOfHats * 4] = {0};

  for (byte i = 0; i < numOfHats * 4; i++) {
    currentHatStates[i] = digitalRead(hatPins[i]);
  }

  for (byte hatIndex = 0; hatIndex < numOfHats; hatIndex++) {
    hatValues[hatIndex] = getHatValue(hatIndex);
  }

switch(hatValues[0]){
    case 1:
      posX = 0;
      posY = -32767;
      break;
    
    case 2:
      posX = 32767;
      posY = -32767;    
      break;
    
    case 3:
      posX = 32767;
      posY = 0;  
      break;

    case 4:
      posX = 32767;
      posY = 32767;  
      break;

    case 5:
      posX = 0;
      posY = 32767;  
      break;

    case 6:
      posX = -32767;
      posY = 32767;  
      break;

    case 7:
      posX = -32767;
      posY = 0;  
      break;

    case 8:
      posX = -32767;
      posY = -32767;  
      break;
    
    default:
      posX = 0;
      posY = 0;  
}
  bleGamepad.setX(posX);
  bleGamepad.setY(posY);

}

signed char getHatValue(byte hatIndex) {
    signed char hatValue = 0;

    for (byte pinIndex = 0; pinIndex < 4; pinIndex++) {
        if (currentHatStates[pinIndex + hatIndex * 4] == LOW) {
            hatValue = pinIndex * 2 + 1;

            if (pinIndex == 0 && currentHatStates[hatIndex * 4 + 3] == LOW) {
                hatValue = 8; // Diagonal superior direita
                break;
            }

            if (pinIndex < 3 && currentHatStates[pinIndex + hatIndex * 4 + 1] == LOW) {
                hatValue += 1; // Diagonal
                break;
            }
        }
    }
    return hatValue;
}

void sendReportIfChanged() {
    if ((memcmp(currentButtonStates, previousButtonStates, sizeof(currentButtonStates)) != 0) ||
        (memcmp(currentHatStates, previousHatStates, sizeof(currentHatStates)) != 0)) {
        
        memcpy(previousButtonStates, currentButtonStates, sizeof(currentButtonStates));
        memcpy(previousHatStates, currentHatStates, sizeof(currentHatStates));
        
        bleGamepad.sendReport(); // Enviar relatório se algum estado mudou
    }
}

void verificaTipoDirecional(){
  primeiroLoop = false;
  int counter = 5000;
  if(digitalRead(19) == LOW && digitalRead(21) == LOW){
    while(digitalRead(19) == LOW && digitalRead(21) == LOW){
      counter = counter - 100;
      delay(100);
    
      if(counter <= 0){
        setDirecionalAnalogico = true;
        return;
      }
    }
  }
}
