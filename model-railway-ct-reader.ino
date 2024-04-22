#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define RED 0xF800
#define GREEN 0x07E0
#define CT_RATIO 1000    // current transformer ratio 1000/1 = 1000
#define SHUNT_RES 40     // shunt resistor connected to CT secondary = 40 Ohm
#define REF_VOLTAGE 1024 // reference voltage for ADC, in millivolts

static const uint8_t PIN_COUNT = 13;
static const uint8_t PIN_ZERO = A0;
static const uint8_t PIN_OFFSET = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// variables
const uint16_t printDelay = 500;
uint64_t lastPrint;
uint16_t i;
uint8_t j, lastPrintedReader = PIN_ZERO + PIN_OFFSET;
const uint16_t samples = 256;
uint16_t r_array[samples];
float readerVals[PIN_COUNT];

void setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 15);
  display.print("ready!");
  display.display();
  delay(3000);

  for (i = 0; i < PIN_COUNT; i++) {
    pinMode(PIN_ZERO + PIN_OFFSET + i, INPUT);
  }
}

void loop() {
  for (i = 0; i < PIN_COUNT; i++) {
    readerVals[i] = read_rms(PIN_ZERO + PIN_OFFSET + i);
  }

  if (millis() - lastPrint > printDelay) {
    lastPrint = millis();

    if (lastPrintedReader++ >= PIN_COUNT) {
      lastPrintedReader = 0;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 15);
    display.print(PIN_ZERO + PIN_OFFSET + i);
    display.print(" ");
    display.print(readerVals[i], 2);
    display.print("mA ");
    if (readerVals[i] > 0)
      display.print("OCC");
    else
      display.print("UNOCC");
    display.display();
  }
}

float read_rms(uint8_t pin) {
  float dc_offset = 0;
  float rms = 0;

  for (i = 0; i < samples; i++) {
    r_array[i] = 0;
  }

  // read voltage at INPUT_CHANNEL 'n' times and save data to 'r_array'
  for (i = 0; i < samples; i++) {
    // adding another 2 bits using oversampling technique
    for (j = 0; j < 16; j++) {
      r_array[i] += digitalRead(pin);
    }
    r_array[i] /= 4;
  }

  // calculate signal average value (DC offset)
  for (i = 0; i < samples; i++) {
    dc_offset += r_array[i];
  }

  dc_offset = dc_offset / samples;

  // calculate AC signal RMS value
  for (i = 0; i < samples; i++) {
    if (abs(r_array[i] - dc_offset) > 3)
      rms += sq(r_array[i] - dc_offset);
  }

  rms = rms / samples;

  // calculate Arduino analog channel input RMS voltage in millivolts
  rms = sqrt(rms) * REF_VOLTAGE /
        4096; // 4096 is max digital value for 12-bit number (oversampled ADC)

  // calculate current passing through the shunt resistor by applying Ohm's Law
  // (in milli Amps)
  rms = rms / SHUNT_RES;
  // now we can get current passing through the CT in milli Amps
  return rms * CT_RATIO;
}