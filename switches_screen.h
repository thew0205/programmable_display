#include "Arial14.h"
#include "Arial_Black_16.h"
#include "DMD2_double_buffer.h"
#include "Droid_Sans_12.h"
#include "SystemFont5x7.h"

#define EEPROM_STARTING_ADDRESS 10
#define EEPROM_STARTING_PASSWORD_ADDRESS 0
#define EEPROM_SPACING_ADDRESS 6
#define EEPROM_TIME_ADDRESS 7
#define EEPROM_SCROLL_DIRECTION_ADDRESS EEPROM_TIME_ADDRESS + 1
#define EEPROM_FONT_ADDRESS EEPROM_SCROLL_DIRECTION_ADDRESS + 1
#define WORD_SIZE_INCREMENTAL (5)
const uint8_t *const FONTS[] PROGMEM = {Arial14, Arial_Black_16, SystemFont5x7,
                                        Droid_Sans_12};
enum class CommandError {
  undefined,
  authorizationFailed,
  invalidCommand,
  invalidIndex,
  invalidDirection,
  passwordLengthError,
  noWord,
  unSuccess,
  invalidTime,
  success
};

struct Pair {
  String key;
  String value;
};

class SwitchesScreen : public Print {
public:
  SPIDMD dmd; // DMD controls the entire display
  int16_t cur_x;
  int16_t cur_y;
  int width;
  int height;
  int left{0};
  uint8_t spacing{0};
  int top{0};
  bool pending_newline{false};
  bool inverted;
  uint8_t scrollDirection{1};
  uint8_t fontIndex;
  size_t write(uint8_t character);
  size_t drawBitPixels(const uint8_t *logoPixel);
  void scrollX(int scrollBy);
  void scrollY(int scrollBy);
  void update(unsigned int elapsedTime);
  void render(uint8_t character);
  void input();
  unsigned long perviousTime = millis();
  void setMemory();
  CommandError parseData(const String &data);
  String incomingData;
  bool incomingDataComplete{false};

  unsigned int wordBufferSize{0};
  unsigned int currentWordIndex{0};
  String *words{NULL};
  unsigned int numberOfWords{0};
  unsigned int currentLetterIndex{0};
  unsigned int currentWordLen{0};
  uint16_t eepromCurrentWordAddress{0};
  String currentWord;
  bool nextWord{false};
  unsigned int delayTime_ms{50};
  bool authorization(const String &password);
  bool appendWord(const String &newWord);
  bool changeWord(const String &newWord, unsigned int index);
  bool insertWord(const String &newWord, unsigned int index);
  bool deleteWord(unsigned int index);
  bool appendFunction(const String &newWord);
  bool changeFunction(const String &newWord, unsigned int index);
  bool insertFunction(const String &newWord, unsigned int index);
  bool deleteFunction(unsigned int index);
  bool setPassword(const String &password);
  bool setDelayTime(const String &delayTime);
  bool setScrollDirection(const String &ScrollDirect);
  bool changeFont(const String &font);
  bool changeSpacing(const String &tempSpacing);

    void reset();
    void clear();
    void swapBuffers();

    void mainLoop();

    bool init();

    SwitchesScreen(uint8_t pixelsWide, uint8_t pixelsHigh);
    ~SwitchesScreen();
  };
static const uint8_t SwitchesLogo[] PROGMEM = {
    0x20, // width
    0x20, // height
    0b00000000, 0b01111111, 0b10000000, 0b00010000, 0b00011111, 0b11111111,
    0b11010000, 0b00110000, 0b00000000, 0b00011111, 0b11111111, 0b00010000,
    0b00000011, 0b11111111, 0b11010100, 0b00010000, 0b00000010, 0b01111111,
    0b10000100, 0b00111000, 0b00000001, 0b00000000, 0b00001000, 0b00000000,
    0b00000000, 0b11111001, 0b11110000, 0b00000000, 0b00000000, 0b00000110,
    0b00000000, 0b00000000, 0b00000000, 0b00000110, 0b00000000, 0b00000000,
    0b00000000, 0b11111001, 0b11110000, 0b00000000, 0b00000001, 0b00000000,
    0b00001000, 0b00000000, 0b00000010, 0b01111111, 0b10000100, 0b00110000,
    0b00000011, 0b11111111, 0b11010100, 0b01001000, 0b00000000, 0b01111111,
    0b11111111, 0b01001000, 0b00011111, 0b11111111, 0b11010000, 0b01001000,
    0b00000000, 0b01111111, 0b10000000, 0b00110000,
};
