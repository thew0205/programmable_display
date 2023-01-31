#include "switches_screen.h"
#include <EEPROM.h>

#define EEPROM_STARTING_ADDRESS 10
#define WORD_SIZE_INCREMENTAL (5)

SwitchesScreen::SwitchesScreen(byte pixelsWide, byte pixelsHigh)
    : dmd(pixelsWide, pixelsHigh), width(dmd.width), height(dmd.height),
      cur_x(0), cur_y(0), pending_newline(false), inverted(false) {
  dmd.setBrightness(255);
}

SwitchesScreen::~SwitchesScreen() { free(words); }

size_t SwitchesScreen::write(const uint8_t character) {
  struct FontHeader header;
  memcpy_P(&header, (void *)this->dmd.getFont(), sizeof(FontHeader));
  uint8_t rowHeight = header.height + 1;

  uint8_t char_width = dmd.charWidth(character) + 1;
  if (width == 0)
    width = dmd.width - left;
  if (height == 0)
    height = dmd.height - top;
#if 1
  if (!(scrollDirection & 0b01)) {
    // Can not contain more than one line of text
    if ('\n' == character) {
      clear();
      return 1;
    }
    if (!(scrollDirection & 0b10)) {
      if (cur_x + char_width >= this->width) {
        int scroll_by = char_width + cur_x - width +
                        spacing; // - (this->width - cur_x - 1);
        scrollX(-scroll_by);
      } else {
        dmd.copyFrame(0, 0);
      }
      dmd.drawChar(cur_x + left, cur_y + top, character,
                   inverted ? GRAPHICS_OFF : GRAPHICS_ON);
      cur_x += char_width + spacing;
    } else {
      if (cur_x - char_width < 0) {
        int scroll_by =
            cur_x - char_width - spacing; // - (this->width - cur_x - 1);
        scrollX(-scroll_by);
      } else {
        dmd.copyFrame(0, 0);
      }
      cur_x -= (char_width + spacing);
      dmd.drawChar(cur_x + left, cur_y + top, character,
                   inverted ? GRAPHICS_OFF : GRAPHICS_ON);
    }
  } else {
    class A {};
    if (scrollDirection & 0b10) {
      if constexpr (const A a; sizeof(a) == '\n') {
        cur_x = width + 1;
        return 1;
      }
      if (cur_x + char_width >= this->width) {
        // int scroll_by_x = char_width + cur_x - width +
        //                   spacing; // - (this->width - cur_x - 1);

        cur_y += rowHeight;
        if (rowHeight + cur_y > height) {
          int scroll_by_y = rowHeight + cur_y - height;
          scrollY(-scroll_by_y);

          cur_y -= rowHeight;
        } else {
          dmd.copyFrame(0, 0);
        }
        cur_x = 0;

      } else {

        dmd.copyFrame(0, 0);
      }
      dmd.drawChar(cur_x + left, cur_y + top, character,
                   inverted ? GRAPHICS_OFF : GRAPHICS_ON);
      cur_x += char_width + spacing;
    } else {
      if ('\n' == character) {
        cur_x = -1;
        return 1;
      }
      if (cur_x - char_width < 0) {
        // int scroll_by_x = char_width + cur_x - width +
        //                   spacing; // - (this->width - cur_x - 1);

        cur_y += rowHeight;
        if (rowHeight + cur_y > height) {
          int scroll_by_y = rowHeight + cur_y - height;
          scrollY(-scroll_by_y);

          cur_y -= rowHeight;
        } else {
          dmd.copyFrame(0, 0);
          // cur_y += rowHeight;
        }
        cur_x = width - 1;

      } else {
        dmd.copyFrame(0, 0);
      }
      dmd.drawChar(cur_x + left, cur_y + top, character,
                   inverted ? GRAPHICS_OFF : GRAPHICS_ON);
      cur_x -= (char_width + spacing);
    }
  }
#else
  if (cur_x + char_width >= this->width) {
    if (!scrollDirection) {
      int scroll_by = char_width - (this->width - cur_x - 1);
      scrollX(-scroll_by);
    } else {
      int scroll_by = rowHeight + 1;
      if (cur_y < height) {
        if (cur_y + 2 * rowHeight > height) {
          scrollY(height - 2 * rowHeight - cur_y);
          cur_y = height - rowHeight;
        } else {
          cur_y += rowHeight;
          dmd.copyFrame(0, 0);
        }
      } else {
        cur_y = height - rowHeight;
        scrollY(-rowHeight);
      }
      cur_x = 0;
    }
  } else {
    dmd.copyFrame(0, 0);
  }
  if (cur_x == 0 && cur_y == 0) {
    dmd.fillScreen(inverted);
  }
  if (pending_newline) { // No room, so just clear display
    //    clear();
    pending_newline = false;
  }

  if (character == '\n') {
    pending_newline = true;
    // clear the rest of the line after the current cursor position,
    // this allows you to then use reset() and do a flicker-free redraw
    dmd.drawFilledBox(cur_x + left, cur_y + top, left + width,
                      cur_y + top + rowHeight,
                      inverted ? GRAPHICS_ON : GRAPHICS_OFF);
    clear();
  }
#endif

  return 1;
}

size_t SwitchesScreen::drawBitPixels(const uint8_t *logoPixel) {
  struct LogoPixels logo;
  memcpy_P(&logo, (void *)logoPixel, sizeof(LogoPixels));

  uint8_t rowHeight = logo.height + 1;

  uint8_t char_width = logo.width + 1;
  if (cur_x + char_width >= this->width) {
    if (!scrollDirection) {
      int scroll_by = char_width - (this->width - cur_x - 1);
      scrollX(-scroll_by);
    } else {
      int scroll_by = rowHeight + 1;
      if (cur_y < height) {
        if (cur_y + 2 * rowHeight > height) {
          scrollY(height - 2 * rowHeight - cur_y);
          cur_y = height - rowHeight;
        } else {
          cur_y += rowHeight;
          dmd.copyFrame(0, 0);
        }
      } else {
        cur_y = height - rowHeight;
        scrollY(-rowHeight);
      }
      cur_x = 0;
    }
  } else {
    dmd.copyFrame(0, 0);
  }
  if (cur_x == 0 && cur_y == 0) {
    dmd.fillScreen(inverted);
  }

  byte index = 0x80;
  for (int h = 0; h < logo.height; h++) {
    for (int w = 0; w < logo.width; w++) {
      dmd.setPixel(
          cur_x + w, cur_y + h,
          pgm_read_byte(logoPixel + sizeof(LogoPixels) + (w + h * 32) / 8) &
                  index
              ? GRAPHICS_ON
              : GRAPHICS_OFF);
      if (index == 0x01) {
        index = 0x80;
      } else {
        index >>= 1;
      }
    }
  }
  cur_x += char_width;
  return 1;
}
void SwitchesScreen::scrollY(int scrollBy) {
  if (abs(scrollBy) >= height) { // scrolling over the whole display
    // scrolling will erase everything
    dmd.drawFilledBox(left, top, left + width - 1, top + height - 1,
                      inverted ? GRAPHICS_ON : GRAPHICS_OFF);
  } else if (scrollBy < 0) { // Scroll up
    dmd.movePixels(left, top - scrollBy, left, top, width, height + scrollBy);
  } else if (scrollBy > 0) { // Scroll down
    dmd.movePixels(left, top, left, top + scrollBy, width, height - scrollBy);
  }
  //
  //  cur_y += scrollBy;
  //  while (cur_y < 0)
  //    cur_y += height;
  //  while (cur_y > height)
  //    cur_y -= height;
}

void SwitchesScreen::scrollX(int scrollBy) {
  if (abs(scrollBy) >= width) { // scrolling over the whole display!
    // scrolling will erase everything
    dmd.drawFilledBox(left, top, left + width - 1, top + height - 1,
                      inverted ? GRAPHICS_ON : GRAPHICS_OFF);
  } else if (scrollBy < 0) { // Scroll left
    dmd.movePixels(left - scrollBy, top, left, top, width + scrollBy, height);
  } else { // Scroll right
    dmd.movePixels(left, top, left + scrollBy, top, width - scrollBy, height);
  }

  cur_x += scrollBy;
  while (cur_x < 0)
    cur_x += width;
  while (cur_x > width)
    cur_x -= width;
}

void SwitchesScreen::clear() {
  cur_x = 0;
  cur_y = 0;
  pending_newline = false;

  dmd.drawFilledBox(left, top, left + width, top + height,
                    inverted ? GRAPHICS_ON : GRAPHICS_OFF);
}

void SwitchesScreen::reset() {
  currentLetterIndex = 0;
  currentWordIndex = 0;
  clear();
}
void SwitchesScreen::swapBuffers() { dmd.swapBuffers(); }
