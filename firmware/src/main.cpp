#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "AQM1248A.h"
#include "GmcDigit12.h"
#include "GmcDigit24.h"

#define PULSE_IN_PIN 2
#define LCD_RS_PIN 3
#define LCD_CS_PIN 4
#define LCD_RST_PIN 5

#define H_MIN 24
#define H_HOUR 12
#define H_H60 12

#define Y_MIN 0
#define Y_HOUR (Y_MIN + H_MIN)
#define Y_H60 (Y_HOUR + H_HOUR)


// CPM → µSv/h 変換係数
#define USVPH_FACTOR_FP 0.0057f

AQM1248A lcd(LCD_CS_PIN, LCD_RS_PIN, LCD_RST_PIN);

// パルスカウント
volatile uint16_t pulseCountWr = 0;
uint16_t pulseCountRd = 0;

// 60 秒 CPM リングバッファ
float s60AveBuff[60];
float s60PeakBuff[60];
uint8_t s60BuffIndex = 0;
uint8_t s60BuffSize = 0;
float s60AveCpm = 0;
float s60PeakCpm = 0;

// 60 分リングバッファ（毎分 lastUsvph を記録）
float m60AveBuff[60];
float m60PeakBuff[60];
uint8_t m60BuffIndex = 0;
uint8_t m60BuffSize = 0;
float m60AveCpm = 0;
float m60PeakCpm = 0;

// 60 時間リングバッファ（毎時 lastHourUsvph を記録）
float h60AveBuff[60];
float h60PeakBuff[60];
uint8_t h60BuffIndex = 0;
uint8_t h60BuffSize = 0;
float h60AveCpm = 0;
float h60PeakCpm = 0;

// タイマー
uint32_t lastSecMillis = 0;
uint32_t lastMinMillis = 0;
uint32_t lastHourMillis = 0;

// ---------------------------------------------------------------------------
// ISR
// ---------------------------------------------------------------------------

void pulseISR() {
  pulseCountWr++;
}

// ---------------------------------------------------------------------------
// 毎秒: CPM・線量更新
// ---------------------------------------------------------------------------

void updateCPM() {
  uint16_t latched = pulseCountWr;

  // リングバッファに今秒のパルス数を書き込む（符号なし減算でカウンタ折り返し自動処理）
  s60AveBuff[s60BuffIndex] = latched - pulseCountRd;
  pulseCountRd = latched;

  if (s60BuffSize < 60) s60BuffSize++;

  float peak = 0;
  float sum = 0;
  for (uint8_t i = 0; i < s60BuffSize; i++) {
    if (s60AveBuff[i] > peak) peak = s60AveBuff[i];
  }
  s60PeakBuff[s60BuffIndex] = peak;

  s60BuffIndex++;
  if (s60BuffIndex >= 60) s60BuffIndex = 0;

  peak = 0;
  for (uint8_t i = 0; i < s60BuffSize; i++) {
    sum += s60AveBuff[i];
    if (s60PeakBuff[i] > peak) peak = s60PeakBuff[i];
  }
  s60AveCpm = sum;
  s60PeakCpm = peak;
}

// ---------------------------------------------------------------------------
// 毎分: 1 時間平均更新
// ---------------------------------------------------------------------------

void updateHour() {
  m60AveBuff[m60BuffIndex] = s60AveCpm;
  m60PeakBuff[m60BuffIndex] = s60PeakCpm;
  m60BuffIndex++;
  if (m60BuffIndex >= 60) m60BuffIndex = 0;
  if (m60BuffSize < 60) m60BuffSize++;

  float sum = 0;
  float peak = 0;
  for (uint8_t i = 0; i < m60BuffSize; i++) {
    sum += m60AveBuff[i];
    if (m60AveBuff[i] > peak) peak = m60AveBuff[i];
  }
  m60AveCpm = (float)sum / m60BuffSize;
  m60PeakCpm = peak;
}

// ---------------------------------------------------------------------------
// 毎時: 60 時間平均更新
// ---------------------------------------------------------------------------

void updateH60() {
  h60AveBuff[h60BuffIndex] = m60AveCpm;
  h60PeakBuff[h60BuffIndex] = m60PeakCpm;
  h60BuffIndex++;
  if (h60BuffIndex >= 60) h60BuffIndex = 0;
  if (h60BuffSize < 60) h60BuffSize++;

  float sum = 0;
  float peak = 0;
  for (uint8_t i = 0; i < h60BuffSize; i++) {
    sum += h60AveBuff[i];
    if (h60AveBuff[i] > peak) peak = h60AveBuff[i];
  }
  h60AveCpm = sum / h60BuffSize;
  h60PeakCpm = peak;
}

// ---------------------------------------------------------------------------
// グラフ描画ヘルパー
// buf      : リングバッファ先頭
// bufSize  : 有効エントリ数
// writeIdx : 次の書き込み位置（最古エントリの直前）
// startY   : グラフ上端 Y
// barHeight: グラフ高さ (px)
// ---------------------------------------------------------------------------

void drawBar(const float* buf, uint8_t bufSize, uint8_t writeIdx,
             uint8_t startY, uint8_t barHeight) {
  if (bufSize == 0) return;

  float maxVal = 0;
  for (uint8_t i = 0; i < bufSize; i++) {
    uint8_t slot = (uint8_t)((writeIdx - bufSize + i + 60u) % 60u);
    if (buf[slot] > maxVal) maxVal = buf[slot];
  }
  if (maxVal == 0) {
    maxVal = 1;
  }

  uint8_t bottomY = startY + barHeight - 1;
  for (uint8_t col = 0; col < bufSize; col++) {
    // 最新データが x=59 になるよう右詰めで配置
    uint8_t x = 60 - bufSize + col;
    uint8_t slot = (uint8_t)((writeIdx - bufSize + col + 60u) % 60u);
    float val = buf[slot];
    uint8_t ph = (uint8_t)(((val * (barHeight - 1)) / maxVal) + 1);
    lcd.drawFastVLine(x, bottomY - ph, ph, 1);
  }
}

void printValue(float value, bool isFloat, int width) {

  uint32_t intPart = (uint32_t)value;
  uint32_t divider = 1;
  uint8_t intDigits = 0;
  while (divider <= intPart) {
    divider *= 10;
    intDigits++;
  }
  if (intDigits == 0) {
    intDigits = 1;
  }

  int8_t fracDigits = 0;
  uint8_t effectiveWidth = intDigits;
  if (isFloat) {
    fracDigits = width - intDigits - 1;
    if (fracDigits < 0) {
      fracDigits = 0;
    }
    effectiveWidth += 1 + fracDigits;  // 小数点分も加算
  }

  for (int i = effectiveWidth; i < width; i++) {
    lcd.print(' ');
  }

  lcd.print(intPart);
  if (isFloat && fracDigits > 0) {

    lcd.print('.');
    // 小数部を表示
    float fracPart = value - intPart;
    for (int i = 0; i < fracDigits; i++) {
      fracPart *= 10;
      uint8_t digit = (uint8_t)fracPart;
      lcd.print(digit);
      fracPart -= digit;
    }
  }
}

// ---------------------------------------------------------------------------
// 数値表示ヘルパー
// aveCpm : 平均 CPM
// peakCpm  : ピーク CPM
// baselineY  : テキスト上端 Y
// font    : 使用するフォント
// ---------------------------------------------------------------------------

void displayValue(float aveCpm, float peakCpm, uint8_t baselineY, const GFXfont* font, bool showPeak, bool isFloat) {
  lcd.setFont(font);
  lcd.setTextColor(1, 0);
  lcd.setCursor(62, baselineY);

  if (false) {
    // 線量で表示
    float aveUsvph = (float)aveCpm * USVPH_FACTOR_FP;
    float peakUsvph = (float)peakCpm * USVPH_FACTOR_FP;
    uint8_t aveIntPart = (uint8_t)aveUsvph;
    uint8_t aveFracPart = (uint8_t)((aveUsvph - aveIntPart) * 100);
    uint8_t peakIntPart = (uint8_t)peakUsvph;
    uint8_t peakFracPart = (uint8_t)((peakUsvph - peakIntPart) * 100);

    if (showPeak) {
      printValue(peakUsvph, true, 4);
    }
    printValue(aveUsvph, true, 4);
  } else {
    // CPM で表示
    if (showPeak) {
      printValue(peakCpm, isFloat, 4);
      lcd.print('/');
    }
    printValue(aveCpm, isFloat, 4);
  }
}

// ---------------------------------------------------------------------------
// 画面更新
// ---------------------------------------------------------------------------

void updateDisplay() {
  lcd.fillScreen(0);

  // 上段 (Y=0..23, 24px): CPM グラフ + 現在線量
  drawBar(s60AveBuff, s60BuffSize, s60BuffIndex, Y_MIN, H_MIN);
  displayValue(s60AveCpm, s60PeakCpm, Y_MIN + H_MIN, &GmcDigit24, false, false);

  // 中段 (Y=24..35, 12px): 1 時間平均グラフ + 1 時間平均
  drawBar(m60AveBuff, m60BuffSize, m60BuffIndex, Y_HOUR + 1, H_HOUR - 1);
  displayValue(m60AveCpm, m60PeakCpm, Y_HOUR + H_HOUR, &GmcDigit12, true, true);

  // 下段 (Y=36..47, 12px): 60 時間グラフ + 60 時間平均
  drawBar(h60AveBuff, h60BuffSize, h60BuffIndex, Y_H60 + 1, H_H60 - 1);
  displayValue(h60AveCpm, h60PeakCpm, Y_H60 + H_H60, &GmcDigit12, true, true);

  lcd.showPic();
}

// ---------------------------------------------------------------------------
// setup / loop
// ---------------------------------------------------------------------------

void setup() {
  memset(s60AveBuff, 0, sizeof(s60AveBuff));
  memset(m60AveBuff, 0, sizeof(m60AveBuff));
  memset(h60AveBuff, 0, sizeof(h60AveBuff));

  lcd.begin();
  lcd.fillScreen(0);
  lcd.showPic();

  pinMode(PULSE_IN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PULSE_IN_PIN), pulseISR, FALLING);

  lastSecMillis = lastMinMillis = lastHourMillis = millis();
}

void loop() {
  uint32_t now = millis();

  if (now - lastSecMillis >= 1000UL) {
    lastSecMillis += 1000UL;
    updateCPM();
    updateDisplay();
  }
  if (now - lastMinMillis >= 60000UL) {
    lastMinMillis += 60000UL;
    updateHour();
  }
  if (now - lastHourMillis >= 3600000UL) {
    lastHourMillis += 3600000UL;
    updateH60();
  }
}
