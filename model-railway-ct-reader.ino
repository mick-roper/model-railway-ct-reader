#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define INPUT_CHANNEL A0
#define CT_RATIO 1000    // current transformer ratio 1000/1 = 1000
#define SHUNT_RES 20     // shunt resistor connected to CT secondary = 20 Ohm
#define REF_VOLTAGE 1024 // reference voltage for ADC, in millivolts
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define RED 0xF800
#define GREEN 0x07E0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// variables
const uint16_t printDelay = 250;
uint64_t lastPrint;
uint16_t i, j;
const uint16_t samples = 256;
uint16_t r_array[samples];
float dc_offset = 0;
float rms = 0;

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

  pinMode(INPUT_CHANNEL, INPUT);
}

void loop() {
  dc_offset = 0;
  rms = 0;

  for (i = 0; i < samples; i++) {
    r_array[i] = 0;
  }

  // read voltage at INPUT_CHANNEL 'n' times and save data to 'r_array'
  for (i = 0; i < samples; i++) {
    // adding another 2 bits using oversampling technique
    for (j = 0; j < 16; j++) {
      r_array[i] += analogRead(INPUT_CHANNEL);
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
  rms = rms * CT_RATIO;

  if (millis() - lastPrint > printDelay) {
    lastPrint = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(5, 12);
    display.print("RMS ");
    display.print(rms, 3);
    display.println("mA");
    display.setCursor(5, 20);
    if (rms > 0)
      display.println("OCCUPIED");
    else
      display.println("UNOCCUPIED");
    display.display();
  }
}