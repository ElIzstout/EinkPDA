//  oooooooooooo         ooooo ooooo      ooo oooo    oooo  //
//  `888'     `8         `888' `888b.     `8' `888   .8P'   //
//   888                  888   8 `88b.    8   888  d8'     //
//   888oooo8    8888888  888   8   `88b.  8   88888[       //
//   888    "             888   8     `88b.8   888`88b.     //
//   888       o          888   8       `888   888  `88b.   //
//  o888ooooood8         o888o o8o        `8  o888o  o888o  //

void refresh() {
  // USE A SLOW FULL UPDATE EVERY N FAST UPDATES OR WHEN SPECIFIED
  if ((partialCounter >= FULL_REFRESH_AFTER) || forceSlowFullUpdate) {
    forceSlowFullUpdate = false;
    partialCounter = 0;
    setFastFullRefresh(false);
  }
  // OTHERWISE USE A FAST FULL UPDATE
  else {
    setFastFullRefresh(true);
    partialCounter++;
  }

  display.display(false);
  display.hibernate();
}

void setFastFullRefresh(bool setting) {
  GxEPD2_310_GDEQ031T10::useFastFullUpdate = setting;
  /*if (GxEPD2_310_GDEQ031T10::useFastFullUpdate != setting) {
    GxEPD2_310_GDEQ031T10::useFastFullUpdate = setting;
  }*/
}

void einkHandler(void* parameter) {
  delay(1000);
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.display(false);
  display.hibernate();

  while (true) {
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    applicationEinkHandler();
  }
}

void statusBar(String input, bool fullWindow) {
  display.setFont(&FreeMonoBold9pt7b);
  if (!fullWindow) display.setPartialWindow(0, display.height() - 20, display.width(), 20);
  display.fillRect(0, display.height() - 26, display.width(), 26, GxEPD_WHITE);
  display.drawRect(0, display.height() - 20, display.width(), 20, GxEPD_BLACK);
  display.setCursor(4, display.height() - 6);
  display.print(input);

  /*switch (CurrentKBState) {
    case NORMAL:
      //Display battery level
      display.drawBitmap(display.width()-30,display.height()-20, KBStatusallArray[6], 30, 20, GxEPD_BLACK);
      break;
    case SHIFT:
      display.drawBitmap(display.width()-30,display.height()-20, KBStatusallArray[0], 30, 20, GxEPD_BLACK);
      break;
    case FUNC:
      display.drawBitmap(display.width()-30,display.height()-20, KBStatusallArray[1], 30, 20, GxEPD_BLACK);
      break;
  }*/
  display.drawRect(display.width() - 30, display.height() - 20, 30, 20, GxEPD_BLACK);
}

void drawStatusBar(String input) {
  display.setFont(&FreeMonoBold9pt7b);
  display.fillRect(0, display.height() - 26, display.width(), 26, GxEPD_WHITE);
  display.drawRect(0, display.height() - 20, display.width(), 20, GxEPD_BLACK);
  display.setCursor(4, display.height() - 6);
  display.print(input);

  // BATTERY INDICATOR
  display.drawBitmap(display.width() - 30, display.height() - 20, KBStatusallArray[battState], 30, 20, GxEPD_BLACK);
  display.drawRect(display.width() - 30, display.height() - 20, 30, 20, GxEPD_BLACK);
}

void setTXTFont(const GFXfont* font) {
  // SET THE FONT
  display.setFont(font);
  currentFont = (GFXfont*)font;

  // UPDATE maxCharsPerLine & maxLines
  maxCharsPerLine = getMaxCharsPerLine();
  maxLines = getMaxLines();
  if (DEBUG_VERBOSE) {
    //Serial.print("char/line: "); Serial.print(maxCharsPerLine);
    //Serial.print("lines: "); Serial.print(maxLines);
    //Serial.println();
  }
}

uint8_t getMaxCharsPerLine() {
  int16_t x1, y1;
  uint16_t charWidth, charHeight;
  // GET AVERAGE CHAR WIDTH
  display.getTextBounds("abcdefghijklmnopqrstuvwxyz", 0, 0, &x1, &y1, &charWidth, &charHeight);
  charWidth = charWidth / 52;

  return (display.width() / (charWidth));
}

uint8_t getMaxLines() {
  int16_t x1, y1;
  uint16_t charWidth, charHeight;
  // GET MAX CHAR HEIGHT
  display.getTextBounds("H", 0, 0, &x1, &y1, &charWidth, &charHeight);
  fontHeight = charHeight;

  return ((display.height() - 26) / (charHeight + lineSpacing));
}

void drawThickLine(int x0, int y0, int x1, int y1, int thickness) {
  float dx = x1 - x0;
  float dy = y1 - y0;
  float length = sqrt(dx * dx + dy * dy);
  float stepX = dx / length;
  float stepY = dy / length;

  for (float i = 0; i <= length; i += thickness / 2.0) {
    int cx = round(x0 + i * stepX);
    int cy = round(y0 + i * stepY);
    display.fillCircle(cx, cy, thickness / 2, GxEPD_BLACK);
  }
}

void einkTextPartial(String text, bool noRefresh) {
  bool doFullRefresh = false;

  einkRefresh++;
  if (einkRefresh > FULL_REFRESH_AFTER) {
    doFullRefresh = true;
    einkRefresh = 0;
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
  }

  display.setFont(&FreeMonoBold9pt7b);

  if (splitIntoLines(text.c_str(), scroll)) doFullRefresh = true;

  for (int i = 0; i < 13; i++) {
    if (outLines[i] != "") {  // Print only non-empty lines
      if (doFullRefresh) {
        display.fillRect(0, 16 * i, display.width(), 16, GxEPD_WHITE);
        display.setCursor(0, 10 + (16 * i));
        display.print(outLines[i]);
      } else if (outLines[i] != lines_prev[i]) {  //If the line has changed
        display.setPartialWindow(0, 16 * i, display.width(), 16);
        display.fillRect(0, 16 * i, display.width(), 16, GxEPD_WHITE);
        display.setCursor(0, 10 + (16 * i));
        display.print(outLines[i]);
        if (!noRefresh) refresh();
      }
    }
  }

  if (doFullRefresh && !noRefresh) {
    display.nextPage();
    display.hibernate();
  }

  for (int i = 0; i < 13; i++) {
    lines_prev[i] = outLines[i];  // Copy each line
  }
}

void einkTextDynamic(bool doFull_, bool noRefresh) {
  // SET MAXIMUMS AND FONT
  setTXTFont(currentFont);

  // ITERATE AND DISPLAY
  uint8_t size = allLines.size();
  uint8_t displayLines = maxLines;

  if (displayLines > size) displayLines = size;  // PREVENT OUT OF BOUNDS

  // FULL REFRESH OPERATION
  if (doFull_) {
    for (uint8_t i = size - displayLines; i < displayLines; i++) {
      if ((allLines[i]).length() > 0) {
        display.setFullWindow();
        display.fillRect(0, (fontHeight + lineSpacing) * i, display.width(), (fontHeight + lineSpacing), GxEPD_WHITE);
        display.setCursor(0, fontHeight + ((fontHeight + lineSpacing) * i));
        display.print(allLines[i]);
        Serial.println(allLines[i]);
      }
    }
  }
  // PARTIAL REFRESH, ONLY SEND LAST LINE
  else {
    if ((allLines[displayLines - 1]).length() > 0) {
      display.setPartialWindow(0, (fontHeight + lineSpacing) * (displayLines - 1), display.width(), (fontHeight + lineSpacing));
      display.fillRect(0, (fontHeight + lineSpacing) * (displayLines - 1), display.width(), (fontHeight + lineSpacing), GxEPD_WHITE);
      display.setCursor(0, fontHeight + ((fontHeight + lineSpacing) * (displayLines - 1)));
      display.print(allLines[displayLines - 1]);
    }
  }

  drawStatusBar("L:" + String(allLines.size()) + "," + editingFile);
}

int countLines(String input, size_t maxLineLength) {
  size_t inputLength = input.length();
  uint8_t charCounter = 0;
  uint16_t lineCounter = 1;

  for (size_t c = 0; c < inputLength; c++) {
    if (input[c] == '\n') {
      charCounter = 0;
      lineCounter++;
      continue;
    } else if (charCounter > (maxLineLength - 1)) {
      charCounter = 0;
      lineCounter++;
    }
    charCounter++;
  }

  return lineCounter;
}