#include <Wire.h>
#include "arduinoFFT.h" // Standard Arduino FFT library
// https://github.com/kosme/arduinoFFT, in IDE, Sketch, Include Library, Manage Library, then search for FFT
arduinoFFT FFT = arduinoFFT();
/////////////////////////////////////////////////////////////////////////
// Comment out the display your nNOT using e.g. if you have a 1.3" display comment out the SSD1306 library and object
//#include "SH1106.h"     // https://github.com/squix78/esp8266-oled-ssd1306
//SH1106 display(0x3c, D3,D4); // 1.3" OLED display object definition (address, SDA, SCL) Connect OLED SDA , SCL pins to ESP SDA, SCL pins


#include "SSD1306.h"  // https://github.com/squix78/esp8266-oled-ssd1306
SSD1306 display(0x3c, D2,D1);  // 0.96" OLED display object definition (address, SDA, SCL) Connect OLED SDA , SCL pins to ESP SDA, SCL pins
/////////////////////////////////////////////////////////////////////////
#define SAMPLES 256              //Must be a power of 2
#define SAMPLING_FREQUENCY 10000 //Hz, must be 10000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT.
#define amplitude 50
unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[] = {0,0,0,0,0,0,0};
int count=0;
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;
/////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  Serial.println("CLEARDATA");
  Serial.println("LABEL,Time,FFT");
  Wire.begin(5,4); // SDA, SCL
  display.init();
  display.setFont(ArialMT_Plain_10);
  display.flipScreenVertically(); // Adjust to suit or remove
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

void loop() {
  display.clear();
  display.drawString(0,0,"0.1 0.2 0.5 1K  2K  4K  8K");
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros()-oldTime;
    oldTime = newTime;
    vReal[i] = analogRead(A0); // A conversion takes about 1mS on an ESP8266
    vImag[i] = 0;
    while (micros() < (newTime + sampling_period_us)) { /* do nothing to wait */ }
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  for (int i = 2; i < (SAMPLES/2); i++){ // Don't use sample 0 and only first SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
    if (vReal[i] > 200) { // Add a crude noise filter, 4 x amplitude or more
      if (i<=5 )           { displayBand(0,(int)vReal[i]/amplitude);} // 125Hz
      if (i >5   && i<=12 )  displayBand(1,(int)vReal[i]/amplitude); // 250Hz
      if (i >12  && i<=32 )  displayBand(2,(int)vReal[i]/amplitude); // 500Hz
      if (i >32  && i<=62 )  displayBand(3,(int)vReal[i]/amplitude); // 1000Hz
      if (i >62  && i<=105 ) displayBand(4,(int)vReal[i]/amplitude); // 2000Hz
      if (i >105 && i<=120 ) displayBand(5,(int)vReal[i]/amplitude); // 4000Hz
      if (i >120 && i<=146 ) displayBand(6,(int)vReal[i]/amplitude); // 8000Hz
    }
    for (byte band = 0; band <= 6; band++) display.drawHorizontalLine(18*band,64-peak[band],14);
  }
  if (millis()%4 == 0) {for (byte band = 0; band <= 6; band++) {if (peak[band] > 0) peak[band] -= 1;}} // Decay the peak
  display.display();
  Serial.println("2초뒤 시작");
  delay(2000);
}

int displayBand(int band, int dsize){
  int dmax = 50;  
  if (dsize > dmax) {dsize=dmax;}
  for (int s = 0; s <= dsize; s=s+2){display.drawHorizontalLine(18*band,64-s, 14); }
  if (dsize > peak[band]) {
    peak[band] = dsize; 
    if(20<= peak[band] && peak[band] <=21){count++;}
    if(23<= peak[band] && peak[band] <=24){count++;}
    if(peak[band] == 26){count++;}
    if(28<= peak[band] && peak[band] <=29){count++;}
    if(32<= peak[band] && peak[band] <=33){count++;}
    if(40<= peak[band] && peak[band] <=41){count++;}
    }
    if(8<count && count<12){    
      if(count==9){int d=70; Serial.print(d); Serial.println("% 일치합니다.");}
      if(count==10){int d=80; Serial.print(d); Serial.println("% 일치합니다.");}
      if(count==11){int d=90; Serial.print(d); Serial.println("% 일치합니다.");} 
      }
      if(count==12){count=0;}
  if (peak[band]>0) {peak[band]=0;}
}
