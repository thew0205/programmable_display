#include "switches_screen.h"
#include "Arial14.h"
#include "SystemFont5x7.h"
#include <EEPROM.h>

#define DEBUG 0

bool updateEEPROM(unsigned int index, uint8_t data) {
  if (index >= 0 && index < EEPROM.length()) {
    EEPROM.update(index, data);
    return true;
  }
  return false;
}
bool readAllEEPROM() {
  for (int index = 0; index < 50; index++) {
    Serial.print(index);
    Serial.print(": ");
    Serial.println(EEPROM.read(index));
  }
  return true;
}


int parsing(const String &incomingData, Pair *infos, int count) {
  Serial.println("parsing");
  String data[count];
  int commaIndex[count];
  for (int index = 0; index < count; index++) {
    if (index == 0) {
      commaIndex[index] = incomingData.indexOf(',', index);
      data[index] = incomingData.substring(index, commaIndex[index]);
      Serial.println(data[index]);

    } else {
      commaIndex[index] = incomingData.indexOf(',', commaIndex[index - 1] + 1);
      data[index] =
        incomingData.substring(commaIndex[index - 1] + 1, commaIndex[index]);
      Serial.println(data[index]);
    }
  }
  int outputcount = 0;
  for (int index = 0; index < count; index++) {
    data[index].trim();

    int indexOfColon = data[index].indexOf(':');
    infos[index].key = data[index].substring(0, indexOfColon);
    infos[index].value = data[index].substring(indexOfColon + 1);
    infos[index].key.trim();
    infos[index].value.trim();
    Serial.print(infos[index].key);
    Serial.print(": ");
    Serial.println(infos[index].value);
    outputcount++;
  }
  return outputcount;
}
bool SwitchesScreen::authorization(const String &password) {
  Serial.println("authorization");
  char authPassword[5] = { 0 };
  for (int i = 0; i < 4; i++) {
    authPassword[i] = (char)EEPROM.read(i);
  }
  Serial.print("password");
  Serial.print(": ");
  Serial.println(authPassword);
  if (strlen(authPassword) == 0 || password == authPassword) {
    return true;
  } else {
    return false;
  }
}
bool SwitchesScreen::setScrollDirection(const String &scrollDirect) {
  Serial.println("setDirection");

  if (scrollDirect.length() > 1)
    return false;
  unsigned int tempDirection = scrollDirect.toInt();
  if (tempDirection >= 0 && tempDirection < 4) {
    Serial.println(
      "..................................................................");
    updateEEPROM(EEPROM_SCROLL_DIRECTION_ADDRESS, tempDirection);
    scrollDirection = tempDirection;
    return true;
  }
  return true;
}

bool SwitchesScreen::setPassword(const String &password) {
  Serial.println("setPassword");

  if (password.length() != 4)
    return false;
  for (int i = 0; i < 4; i++) {
    updateEEPROM(EEPROM_STARTING_PASSWORD_ADDRESS + i, password[i]);
  }
  return true;
}

bool SwitchesScreen::setDelayTime(const String &timeDelay) {
  Serial.println("setDelayTime");
  int delayTime = timeDelay.toInt();
  if (delayTime > 0 && delayTime < 5000) {
    updateEEPROM(EEPROM_TIME_ADDRESS, (uint8_t)delayTime / 20);
    delayTime_ms = delayTime;
    return true;
  }
  return false;
}

bool SwitchesScreen::changeFont(const String &tempFont) {
  uint8_t font = tempFont.toInt();
  Serial.println("changeFont");
  //  Serial.println(sizeof(FONTS));
  if (font > 0 && font < 5) {
    fontIndex = font - 1;
    updateEEPROM(EEPROM_FONT_ADDRESS, (uint8_t)(fontIndex));
    // init();
    //    dmd.selectFont((uint8_t *)pgm_read_word(&(FONTS[fontIndex])));
    return true;
  }
  return false;
}

bool SwitchesScreen::changeSpacing(const String &tempSpacing) {
  uint8_t space = tempSpacing.toInt();
  Serial.println("changeSpacing");
  //  Serial.println(sizeof(FONTS));
  if (space >= 0 && space < 15) {
    spacing = space;
    updateEEPROM(EEPROM_SPACING_ADDRESS, (uint8_t)(space));
    // init();
    //    dmd.selectFont((uint8_t *)pgm_read_word(&(FONTS[fontIndex])));
    return true;
  }
  return false;
}

CommandError SwitchesScreen::parseData(const String &incomingData) {
  // data example k:1234,c:a,w:matthew
  Serial.println("parseData");

  // parsing the data
  int count = 0;

  int from = 0;

  while (from != -1) {
    if (from == 0) {
      from = incomingData.indexOf(',', from);
    } else {
      from = incomingData.indexOf(',', from + 1);
    }
    Serial.println(from);
    count++;
  }
  if (count < 3)
    return CommandError::undefined;

    //  from = 0;
#if DEBUG
  Serial.print("count");
  Serial.print(": ");
  Serial.println(count);
#endif
  Pair infos[count];

  {
    if (count != parsing(incomingData, infos, count)) {
      Serial.println(parsing(incomingData, infos, count));
    }
  }

  if (infos[0].key == "k" && authorization(infos[0].value)) {
    if (infos[1].key == "c") {
      if (infos[1].value == "a") {
        if (infos[2].key == "w") {
          CommandError error = appendFunction(infos[2].value)
                                 ? CommandError::success
                                 : CommandError::noWord;
          return error;
        } else {
          return CommandError::undefined;
        }
      } else if (infos[1].value == "c") {
        if (infos[2].key == "i" && infos[3].key == "w") {

          unsigned int position = infos[2].value.toInt();
          if (position > 0 && position <= numberOfWords) {
            CommandError error = changeFunction(infos[3].value, position - 1)
                                   ? CommandError::success
                                   : CommandError::unSuccess;
            return error;
          } else {
            return CommandError::invalidIndex;
          }
        } else {
          return CommandError::undefined;
        }
      } else if (infos[1].value == "i") {
        if (infos[2].key == "i" && infos[3].key == "w") {

          unsigned int position = infos[2].value.toInt();
          if (position > 0 && position <= numberOfWords) {
            CommandError error = insertFunction(infos[3].value, position - 1)
                                   ? CommandError::success
                                   : CommandError::unSuccess;
            return error;
          } else {
            return CommandError::invalidIndex;
          }
        } else {
          return CommandError::undefined;
        }
      } else if (infos[1].value == "d") {
        if (infos[2].key == "i") {

          unsigned int position = infos[2].value.toInt();
          if (position > 0 && position <= numberOfWords) {
            CommandError error = deleteFunction(position - 1)
                                   ? CommandError::success
                                   : CommandError::unSuccess;
            return error;
          } else {
            return CommandError::invalidIndex;
          }
        } else {
          return CommandError::undefined;
        }
      } else if (infos[1].value == "p") {
        if (infos[2].key == "p") {
          if (infos[2].value.length() != 4) {
            return CommandError::passwordLengthError;
          }
          CommandError error = setPassword(infos[2].value)
                                 ? CommandError::success
                                 : CommandError::unSuccess;
          return error;
          ;
        } else {
          return CommandError::undefined;
        }
      } else if (infos[1].value == "t") {
        if (infos[2].key == "t") {
          Serial.println(infos[2].value);
          CommandError error = setDelayTime(infos[2].value)
                                 ? CommandError::success
                                 : CommandError::invalidTime;
          return error;
        } else {
          return CommandError::undefined;
        }
      } else if (infos[1].value == "s") {
        if (infos[2].key == "s") {
          Serial.println(infos[2].value);
          CommandError error = setScrollDirection(infos[2].value)
                                 ? CommandError::success
                                 : CommandError::invalidDirection;
          return error;
        } else {
          return CommandError::undefined;
        }
      } else if (infos[1].value == "f") {
        if (infos[2].key == "f") {
          Serial.println(infos[2].value);
          CommandError error = changeFont(infos[2].value)
                                 ? CommandError::success
                                 : CommandError::invalidDirection;
          return error;
        } else {
          return CommandError::undefined;
        }
      } else if (infos[1].value == "n") {
        if (infos[2].key == "n") {
          Serial.println(infos[2].value);
          CommandError error = changeSpacing(infos[2].value)
                                 ? CommandError::success
                                 : CommandError::invalidDirection;
          return error;
        } else {
          return CommandError::undefined;
        }
      } else {
        return CommandError::invalidCommand;
      }

    } else {
      return CommandError::undefined;
    }

  } else {
    return CommandError::authorizationFailed;
  }
}

bool SwitchesScreen::appendFunction(const String &newWord) {
  Serial.println("appendFunction");
  bool success = false;
  if (newWord.length() > 0) {
    success = appendWord(newWord);
    return success;
  } else {
    return false;
  }
}
bool SwitchesScreen::changeFunction(const String &newWord, unsigned int index) {
  Serial.println("changeFunction");
  if (newWord.length() > 0) {
    return changeWord(newWord, index);
  }
  return false;
}

bool SwitchesScreen::insertFunction(const String &newWord, unsigned int index) {
  Serial.println("insertFunction");
  if (newWord.length() > 0) {
    return insertWord(newWord, index);
  }
  return false;
}

bool SwitchesScreen::deleteFunction(unsigned int index) {
  Serial.println("deleteFunction");
  return deleteWord(index);
}

bool SwitchesScreen::appendWord(const String &newWord) {
  Serial.println("appendWord");
  // if (NULL == words) {
  //   if (!init())
  //     return false;
  // }

  unsigned int tempEepromAddress = EEPROM_STARTING_ADDRESS;
  tempEepromAddress++;
  for (uint8_t i = 0; i < numberOfWords; i++) {
#if DEBUG
    Serial.print(tempEepromAddress);
    Serial.print(": ");
    Serial.println((uint8_t)EEPROM.read(tempEepromAddress));
#endif
    tempEepromAddress += (uint8_t)EEPROM.read(tempEepromAddress);
    tempEepromAddress++;
  }
  updateEEPROM(tempEepromAddress, (uint8_t)newWord.length());
  tempEepromAddress++;
  for (uint8_t i = 0; i < newWord.length(); i++) {
#if DEBUG
    Serial.print(tempEepromAddress);
    Serial.print(": ");
    Serial.println((uint8_t)EEPROM.read(newWord[i]));
#endif
    if (!updateEEPROM(tempEepromAddress, newWord[i])) {
      return false;
    }
    tempEepromAddress++;
  }
  numberOfWords++;
  updateEEPROM(EEPROM_STARTING_ADDRESS, numberOfWords);
  // if (numberOfWords <= wordBufferSize) {
  //   words[numberOfWords] = newWord;
  //   Serial.println(words[numberOfWords]);
  //   numberOfWords++;
  //   setMemory();
  //   Serial.println(numberOfWords);
  //   return true;

  // } else {
  //   wordBufferSize += WORD_SIZE_INCREMENTAL;
  //   String *newWords =
  //       (String *)realloc(words, wordBufferSize * sizeof(String));
  //   if (NULL == words) {
  //     return false;
  //   } else {
  //     words = newWords;
  //     words[numberOfWords] = newWord;
  //     numberOfWords++;
  //     setMemory();
  //     return true;
  //   }
  // }
  return true;
}

bool SwitchesScreen::changeWord(const String &newWord, unsigned int index) {
  if (index < numberOfWords && index >= 0) {
    uint16_t tempEepromAddress = EEPROM_STARTING_ADDRESS + 1;
    Serial.println("..............................");
    for (uint8_t i = 0; i < index; i++) {
#if DEBUG
      Serial.print(tempEepromAddress);
      Serial.print(": ");
      Serial.println((uint8_t)EEPROM.read(tempEepromAddress));
#endif
      tempEepromAddress += (uint8_t)EEPROM.read(tempEepromAddress);

      tempEepromAddress++;
    }

    Serial.println("..............................");
    uint16_t addressOfTheWordsToBeMoved = tempEepromAddress;
    uint16_t addressOfWhereTheWordsIsToBeMoved =
      tempEepromAddress + newWord.length();

    addressOfWhereTheWordsIsToBeMoved++;
    tempEepromAddress += (uint8_t)EEPROM.read(tempEepromAddress);
    tempEepromAddress++;
    uint8_t remainingWords = numberOfWords - index - 1;
#if DEBUG
    Serial.print("tempEepromAddress");
    Serial.print(": ");
    Serial.println(tempEepromAddress);
    Serial.print("addressOfTheWordsToBeMoved");
    Serial.print(": ");
    Serial.println(addressOfTheWordsToBeMoved);
    Serial.print("addressOfWhereTheWordsIsToBeMoved");
    Serial.print(": ");
    Serial.println(addressOfWhereTheWordsIsToBeMoved);

#endif
    uint16_t lenOfThePerviousWordToBeMoved =
      (uint8_t)EEPROM.read(tempEepromAddress);
    for (uint8_t i = 0; i < remainingWords - 1; i++) {
      lenOfThePerviousWordToBeMoved = (uint8_t)EEPROM.read(tempEepromAddress);
      tempEepromAddress += lenOfThePerviousWordToBeMoved;
      addressOfWhereTheWordsIsToBeMoved += lenOfThePerviousWordToBeMoved;
      addressOfWhereTheWordsIsToBeMoved++;
      tempEepromAddress++;
      //      Serial.print("tempEepromAddress");
      //      Serial.print(": ");
      //      Serial.println(tempEepromAddress);
      //      Serial.print("addressOfWhereTheWordsIsToBeMoved");
      //      Serial.print(": ");
      //      Serial.println(addressOfWhereTheWordsIsToBeMoved);
    }

    for (uint8_t i = 0; i < remainingWords; i++) {
      readAllEEPROM();
      int curWordLen = EEPROM.read(tempEepromAddress);
      uint8_t currWord[curWordLen];

      for (uint8_t j = 0; j < curWordLen; j++) {
        currWord[j] = EEPROM.read(tempEepromAddress + j + 1);
      }

      updateEEPROM(addressOfWhereTheWordsIsToBeMoved,
                   EEPROM.read(tempEepromAddress));

      updateEEPROM(tempEepromAddress, curWordLen);
      tempEepromAddress;
      for (uint8_t j = 0; j < curWordLen; j++) {
        updateEEPROM(addressOfWhereTheWordsIsToBeMoved + j + 1, currWord[j]);
      }
      addressOfWhereTheWordsIsToBeMoved -= lenOfThePerviousWordToBeMoved;
      tempEepromAddress -= lenOfThePerviousWordToBeMoved;
      addressOfWhereTheWordsIsToBeMoved--;
      tempEepromAddress--;
      lenOfThePerviousWordToBeMoved = (uint8_t)EEPROM.read(tempEepromAddress);
    }
#if DEBUG
    Serial.print("tempEepromAddress");
    Serial.print(": ");
    Serial.println(tempEepromAddress);
    Serial.print("addressOfTheWordsToBeMoved");
    Serial.print(": ");
    Serial.println(addressOfTheWordsToBeMoved);
    Serial.print("addressOfWhereTheWordsIsToBeMoved");
    Serial.print(": ");
    Serial.println(addressOfWhereTheWordsIsToBeMoved);
    delay(1000);
#endif
    updateEEPROM(addressOfTheWordsToBeMoved, (uint8_t)newWord.length());
    addressOfTheWordsToBeMoved++;
    for (uint8_t i = 0; i < newWord.length(); i++) {
      //      Serial.print(tempEepromAddress);
      //      Serial.print(": ");
      //      Serial.println((uint8_t)EEPROM.read( newWord[i]));
      if (!updateEEPROM(addressOfTheWordsToBeMoved, newWord[i])) {
        return false;
      }
      addressOfTheWordsToBeMoved++;
    }
    while (addressOfWhereTheWordsIsToBeMoved <= tempEepromAddress) {
      updateEEPROM(addressOfWhereTheWordsIsToBeMoved, 0);
      addressOfWhereTheWordsIsToBeMoved++;
    }
    //    if (tempEepromAddress != addressOfTheWordsToBeMoved)abort();
    //    if (index < numberOfWords && index >= 0) {
    //
    //      words[index] = newWord;
    //      setMemory();
    //      return true;
    //    }
    return true;
  }
  return false;
}

bool SwitchesScreen::insertWord(const String &newWord, unsigned int index) {
  if (index < numberOfWords && index >= 0) {
    uint16_t tempEepromAddress = EEPROM_STARTING_ADDRESS + 1;
    Serial.println("..............................");
    for (uint8_t i = 0; i < index; i++) {
#if DEBUG
      Serial.print(tempEepromAddress);
      Serial.print(": ");
      Serial.println((uint8_t)EEPROM.read(tempEepromAddress));
#endif
      tempEepromAddress += (uint8_t)EEPROM.read(tempEepromAddress);

      tempEepromAddress++;
    }

    Serial.println("..............................");
    uint16_t addressOfTheWordsToBeMoved = tempEepromAddress;
    uint16_t addressOfWhereTheWordsIsToBeMoved =
      tempEepromAddress + newWord.length();
    addressOfWhereTheWordsIsToBeMoved++;
    uint8_t remainingWords = numberOfWords - index;
#if DEBUG
    Serial.print("tempEepromAddress");
    Serial.print(": ");
    Serial.println(tempEepromAddress);
    Serial.print("addressOfTheWordsToBeMoved");
    Serial.print(": ");
    Serial.println(addressOfTheWordsToBeMoved);
    Serial.print("addressOfWhereTheWordsIsToBeMoved");
    Serial.print(": ");
    Serial.println(addressOfWhereTheWordsIsToBeMoved);
#endif
    uint16_t lenOfThePerviousWordToBeMoved =
      (uint8_t)EEPROM.read(tempEepromAddress);
    for (uint8_t i = 0; i < remainingWords - 1; i++) {
      lenOfThePerviousWordToBeMoved = (uint8_t)EEPROM.read(tempEepromAddress);
      tempEepromAddress += lenOfThePerviousWordToBeMoved;
      addressOfWhereTheWordsIsToBeMoved += lenOfThePerviousWordToBeMoved;
      addressOfWhereTheWordsIsToBeMoved++;
      tempEepromAddress++;
#if DEBUG
      Serial.print("tempEepromAddress");
      Serial.print(": ");
      Serial.println(tempEepromAddress);
      Serial.print("addressOfWhereTheWordsIsToBeMoved");
      Serial.print(": ");
      Serial.println(addressOfWhereTheWordsIsToBeMoved);
#endif
    }

    for (uint8_t i = 0; i < remainingWords; i++) {
      readAllEEPROM();
      int curWordLen = EEPROM.read(tempEepromAddress);
      uint8_t currWord[curWordLen];

      for (uint8_t j = 0; j < curWordLen; j++) {
        currWord[j] = EEPROM.read(tempEepromAddress + j + 1);
      }

      updateEEPROM(addressOfWhereTheWordsIsToBeMoved,
                   EEPROM.read(tempEepromAddress));

      updateEEPROM(tempEepromAddress, curWordLen);
      tempEepromAddress;
      for (uint8_t j = 0; j < curWordLen; j++) {
        updateEEPROM(addressOfWhereTheWordsIsToBeMoved + j + 1, currWord[j]);
      }
      addressOfWhereTheWordsIsToBeMoved -= lenOfThePerviousWordToBeMoved;
      tempEepromAddress -= lenOfThePerviousWordToBeMoved;
      addressOfWhereTheWordsIsToBeMoved--;
      tempEepromAddress--;
      lenOfThePerviousWordToBeMoved = (uint8_t)EEPROM.read(tempEepromAddress);
    }

#if DEBUG
    Serial.print("tempEepromAddress");
    Serial.print(": ");
    Serial.println(tempEepromAddress);
    Serial.print("addressOfTheWordsToBeMoved");
    Serial.print(": ");
    Serial.println(addressOfTheWordsToBeMoved);
    Serial.print("addressOfWhereTheWordsIsToBeMoved");
    Serial.print(": ");
    Serial.println(addressOfWhereTheWordsIsToBeMoved);
    delay(1000);
#endif
    updateEEPROM(addressOfTheWordsToBeMoved, (uint8_t)newWord.length());
    addressOfTheWordsToBeMoved++;
    for (uint8_t i = 0; i < newWord.length(); i++) {
      //      Serial.print(tempEepromAddress);
      //      Serial.print(": ");
      //      Serial.println((uint8_t)EEPROM.read( newWord[i]));
      if (!updateEEPROM(addressOfTheWordsToBeMoved, newWord[i])) {
        return false;
      }
      addressOfTheWordsToBeMoved++;
    }
    numberOfWords++;
    updateEEPROM(EEPROM_STARTING_ADDRESS, numberOfWords);
    //    if (tempEepromAddress != addressOfTheWordsToBeMoved)abort();
    //    if (index < numberOfWords && index >= 0) {
    //
    //      words[index] = newWord;
    //      setMemory();
    //      return true;
    //    }
    return true;
  }
  return false;
}
bool SwitchesScreen::deleteWord(unsigned int index) {

  if (index < numberOfWords && index >= 0) {
    unsigned int tempEepromAddress = EEPROM_STARTING_ADDRESS + 1;

    for (uint8_t i = 0; i < index; i++) {

      tempEepromAddress += (uint8_t)EEPROM.read(tempEepromAddress);

      tempEepromAddress++;
    }
    uint16_t addressOfTheWordToBeRemoved = tempEepromAddress;
    tempEepromAddress += (uint8_t)EEPROM.read(tempEepromAddress);
    tempEepromAddress++;
    uint8_t remainingWords = numberOfWords - index - 1;
#if DEBUG
    Serial.print("remaining words");
    Serial.print(": ");
    Serial.println(remainingWords);
#endif
    for (uint8_t i = 0; i < remainingWords; i++) {
      int curWordLen = EEPROM.read(tempEepromAddress);
      updateEEPROM(addressOfTheWordToBeRemoved, curWordLen);
      addressOfTheWordToBeRemoved++;
      tempEepromAddress++;
#if DEBUG
      Serial.print(tempEepromAddress);
      Serial.print(": ");
      Serial.println((uint8_t)EEPROM.read(tempEepromAddress));
#endif
      for (uint8_t j = 0; j < curWordLen; j++) {
        updateEEPROM(addressOfTheWordToBeRemoved,
                     EEPROM.read(tempEepromAddress));
        addressOfTheWordToBeRemoved++;
        tempEepromAddress++;
      }
    }
    while (addressOfTheWordToBeRemoved <= tempEepromAddress) {
      updateEEPROM(addressOfTheWordToBeRemoved, 0);
      addressOfTheWordToBeRemoved++;
    }
    numberOfWords--;
    updateEEPROM(EEPROM_STARTING_ADDRESS, numberOfWords);
  }
  return true;
}

void clearMemory(int start = 0, int end = EEPROM.length()) {
  for (; start < end; start++) {
    updateEEPROM(start, 0);
  }
}
void printError(CommandError error) {
  Serial.println("printError");
  switch (error) {
    case CommandError::undefined:
      Serial.println("undefined");
      break;
    case CommandError::authorizationFailed:
      Serial.println("authorizationFailed");
      break;
    case CommandError::invalidCommand:
      Serial.println("invalidcommand");
      break;
    case CommandError::invalidIndex:
      Serial.println("invalidIndex");
      break;
      Serial.println("invalidTime");
      break;
    case CommandError::noWord:
      Serial.println("noWord");
      break;
    case CommandError::invalidDirection:
      Serial.println("invalidDirection");
      break;
    case CommandError::passwordLengthError:
      Serial.println("passwordLengthError");
      break;
    case CommandError::success:
      Serial.println("success");
      break;
    case CommandError::unSuccess:
      Serial.println("unsuccess");
      break;
  }
}

void (*resetFunc)(void) = 0;

void SwitchesScreen::setMemory() {
  Serial.println("setMemory");

  Serial.print("numberOfWords");
  Serial.print(": ");
  Serial.println(numberOfWords);

  updateEEPROM(EEPROM_STARTING_ADDRESS, (uint8_t)numberOfWords);
  for (unsigned int i = 1; i <= numberOfWords; i++) {
    updateEEPROM(EEPROM_STARTING_ADDRESS + i, words[i - 1].length());
    Serial.print(EEPROM_STARTING_ADDRESS + i);
    Serial.print(": ");
    Serial.println(words[i - 1].length());
  }
  unsigned int addressPosition = EEPROM_STARTING_ADDRESS + numberOfWords + 1;
  for (unsigned int i = 0; i < numberOfWords; i++) {
    for (unsigned int j = 0; j < words[i].length(); j++) {
      unsigned int address = addressPosition + j;
      updateEEPROM(address, words[i][j]);
      Serial.print(address);
      Serial.print(": ");
      Serial.println(words[i][j]);
    }
    addressPosition += words[i].length();
    Serial.print("addressPosition");
    Serial.print(": ");
    Serial.println(addressPosition);
  }
}

bool SwitchesScreen::init() {
  Serial.println("init");
  // clearMemory();
  dmd.clearScreen();
  if (EEPROM.read(EEPROM_TIME_ADDRESS) == 0) {
    delayTime_ms = 10 * 20;
  } else {
    delayTime_ms = (EEPROM.read(EEPROM_TIME_ADDRESS) + 1) * 20;
  }

  scrollDirection = EEPROM.read(EEPROM_SCROLL_DIRECTION_ADDRESS);
  fontIndex = EEPROM.read(EEPROM_FONT_ADDRESS);
  cur_x = 0;
  cur_y = 0;
  spacing = EEPROM.read(EEPROM_SPACING_ADDRESS);
  numberOfWords = (uint8_t)EEPROM.read(EEPROM_STARTING_ADDRESS);
  if (numberOfWords > 0) {
    eepromCurrentWordAddress = EEPROM_STARTING_ADDRESS + 1;
    currentWordLen = (int)EEPROM.read(eepromCurrentWordAddress);
    eepromCurrentWordAddress++;
    char letters[currentWordLen + 1] = { 0 };
    //    Serial.println(eepromCurrentWordAddress);
    for (int j = 0; j < currentWordLen; j++) {
      letters[j] = EEPROM.read(eepromCurrentWordAddress);
      eepromCurrentWordAddress++;
      //      Serial.print(letters[j]);
      //      Serial.print(": ");
      //      Serial.println(eepromCurrentWordAddress);
    }
    currentWordIndex = 0;
    currentLetterIndex = 0;
    currentWord = letters;
  }

#if 1
  Serial.print("numberOfWords");
  Serial.print(": ");
  Serial.println(numberOfWords);
#endif
  // uint8_t wordLength[numberOfWords];
  // for (unsigned int i = 1; i <= numberOfWords; i++) {
  //   wordLength[i - 1] = EEPROM.read(EEPROM_STARTING_ADDRESS + i);
  //   Serial.print(wordLength[i - 1]);
  //   Serial.print(": ");
  //   Serial.println(EEPROM_STARTING_ADDRESS + i);
  // }
  // int addressPosition = EEPROM_STARTING_ADDRESS + numberOfWords + 1;
  // for (unsigned int i = 0; i < numberOfWords; i++) {
  //   char letters[wordLength[i] + 1] = {0};
  //   for (int j = 0; j < wordLength[i]; j++) {
  //     int address = addressPosition + j;
  //     letters[j] = EEPROM.read(address);
  //     Serial.print(letters[j]);
  //     Serial.print(": ");
  //     Serial.println(address);
  //   }
  //   words[i] = letters;
  //   addressPosition += wordLength[i];
  //   Serial.print("addressPosition");
  //   Serial.print(": ");
  //   Serial.println(addressPosition);
  // }
  dmd.selectFont((uint8_t *)pgm_read_word(&(FONTS[fontIndex])));
  // dmd.clearScreen();
  // swapBuffers();
  dmd.begin();
  //  readAllEEPROM();
  //  appendWord("Help me");
  //  appendWord("Happy");
  //  //    deleteWord(4);
  //  appendWord("matthew");
  //  appendWord("Tolulope Busoye");
  //  readAllEEPROM();
  //
  //    deleteWord(2);
  //  readAllEEPROM();
  //  changeWord("Tolulope", 2);
  //  insertWord("Matthew", 3);
  readAllEEPROM();
  Serial.println("endinit");
  return true;
  // if (words != NULL) {
  //   return true;
  // } else {
  //   return false;
  // }
}

void SwitchesScreen::mainLoop() {
  input();
  unsigned int elapsedTime = millis() - perviousTime;
  if (elapsedTime > delayTime_ms) {

    update(elapsedTime);
    perviousTime = millis();
  }
}

void SwitchesScreen::update(unsigned int elapsedTime) {
  static uint8_t delayNextWord = 1;

  if (numberOfWords == 0) {
    if (!nextWord) {
      if (currentLetterIndex < currentWordLen) {
        render(currentWord.charAt(currentLetterIndex));
        currentLetterIndex++;
      } else {
        nextWord = true;
        delayNextWord = 1;

        currentWordIndex = 0;
        currentLetterIndex = 0;
        currentWord = "Input word to display";
        currentWordLen = currentWord.length();
      }
    } else {
      if (delayNextWord % 8 != 0) {
        delayNextWord++;
      } else {
        render('\n');
        nextWord = false;
        // delay(100);
      }
    }


  } else {
    if (!nextWord) {
      if (currentWordIndex == 0 && currentLetterIndex == 0) {
        render('\n');
      }
      if (currentWordIndex < numberOfWords) {

        if (currentLetterIndex < currentWordLen) {
          render(currentWord.charAt(currentLetterIndex));
          currentLetterIndex++;
        } else {
          nextWord = true;
          delayNextWord = 1;
          currentLetterIndex = 0;
          currentWordIndex++;
          currentWordLen = (int)EEPROM.read(eepromCurrentWordAddress);
          eepromCurrentWordAddress++;
          char letters[currentWordLen + 1] = { 0 };
          for (int j = 0; j < currentWordLen; j++) {
            letters[j] = EEPROM.read(eepromCurrentWordAddress);

            //          Serial.print(letters[j]);
            //          Serial.print(": ");
            //          Serial.println(currentWordIndex);
            eepromCurrentWordAddress++;
          }
          currentWord = letters;
        }
      } else {
        currentWordIndex = 0;
        currentLetterIndex = 0;
        eepromCurrentWordAddress = EEPROM_STARTING_ADDRESS + 1;
        currentWordLen = (int)EEPROM.read(eepromCurrentWordAddress);
        eepromCurrentWordAddress++;
        char letters[currentWordLen + 1] = { 0 };
        for (int j = 0; j < currentWordLen; j++) {
          letters[j] = EEPROM.read(eepromCurrentWordAddress);
          //
          //        Serial.print(letters[j]);
          //        Serial.print(": ");
          //        Serial.println(currentWordLen);
          eepromCurrentWordAddress++;
        }
        currentWord = letters;
      }
    } else {

      if (delayNextWord % 8 != 0) {
        delayNextWord++;
      } else {
        render('\n');
        nextWord = false;
        // delay(100);
      }
    }
  }



  // if (numberOfWords == 0) {
  //   currentWordIndex = 0;
  //   currentLetterIndex = 0;
  //   // eepromCurrentWordAddress = EEPROM_STARTING_ADDRESS + 1;
  //   currentWord = "Welcome, input word to display";
  //   currentWordLen = currentWord.length();
  // }


  // if (!nextWord) {

  //   if (currentWordIndex < numberOfWords) {
  //     if (currentLetterIndex < words[currentWordIndex].length()) {
  //       render(words[currentWordIndex].charAt(currentLetterIndex));
  //       currentLetterIndex++;
  //     } else {
  //       nextWord = true;
  //       delayNextWord = 1;
  //       currentWordIndex++;
  //       currentLetterIndex = 0;
  //     }
  //   } else {
  //     currentWordIndex = 0;
  //     currentLetterIndex = 0;
  //   }
  // } else {

  //   if (delayNextWord % 8 != 0) {
  //     delayNextWord++;
  //   } else {
  //     render('\n');
  //     nextWord = false;
  //   }
  // }
}

void SwitchesScreen::render(uint8_t character) {
  //  Serial.print("currentWordIndex");
  //  Serial.println(currentWordIndex);
  //  Serial.print("currentLetterIndex");
  //  Serial.println(currentLetterIndex);
  if (character == '~') {
    drawBitPixels(SwitchesLogo);
  } else {
    write(character);
  }
  swapBuffers();
}

void SwitchesScreen::input() {
  int availableData = Serial.available();
  if (availableData) {
    char dataChar[availableData + 1] = { 0 };
    for (int i = 0; i < availableData; i++) {
      dataChar[i] = Serial.read();
      if (dataChar[i] == '\r') {
        incomingDataComplete = true;
        Serial.println(dataChar);
        break;
      }
    }
    incomingData += dataChar;
    Serial.print("incomingData");
    Serial.print(": ");
    Serial.println(incomingData);
    incomingData.trim();
    if (incomingDataComplete) {
      // parse data
      auto error = parseData(incomingData);
      printError(error);

      if (CommandError::success == error) {
        init();
      }
      incomingDataComplete = false;
      incomingData.remove(0);
    };
  }
}
