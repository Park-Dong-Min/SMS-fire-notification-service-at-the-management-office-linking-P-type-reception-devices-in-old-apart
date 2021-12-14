## 노후 아파트의 P형 수신 장치를 연동하는 관리사무소 SMS 화재 알림 서비스
### 개발자 : 박동민
### 진행기간 : 2020-12-22 ~ 2021-01-08

### 사용기술
---
+ <img src="https://img.shields.io/badge/C-A8B9CC?style=flat-square&logo=C&logoColor=white"/> <img src="https://img.shields.io/badge/Arduino-00979D?style=flat-square&logo=Arduino&logoColor=white"/> - 개발언어
+ <img src="https://img.shields.io/badge/GitHub-181717?style=flat-square&logo=GitHub&logoColor=white"/> - 형상관리
+ <img src="https://img.shields.io/badge/Microsoft Excel-217346?style=flat-square&logo=Microsoft Excel&logoColor=white"/> - 데이터 저장

### 설명
---
- 동계 현장실습에서 진행했던 프로젝트입니다.
- 화재 발생시 관리자가 자리에 없어서 발생하는 피해를 방지하는 시스템입니다.
- 조건을 만족하면 관리자에게 화재경보 문자를 발송합니다.

### 기능
---
- 경종 소리를 입력받아 샘플링 및 FFT로 변환하여 평균값 계산
- 소리가 평균값에 비슷할 경우 경종 소리로 인식
- 화재경보 문자를 관리자에게 발송

### 개발 기획
---
- 경비원의 수가 줄어들어 야간 혹은 주말에 순찰을 위해 자리를 비웠을 때 경종 소리가 발생하면 즉각적인 조치가 어렵기 때문에 자리에 없더라도 관리자에게 화재가 발생했다고 알려줄 필요가 있다.


## 소스코드
### Sound_senser_V2_Serial_and_excel_save
```C
#include <Wire.h>
#include "arduinoFFT.h" // Standard Arduino FFT library
// https://github.com/kosme/arduinoFFT, in IDE, Sketch, Include Library, Manage Library, then search for FFT
arduinoFFT FFT = arduinoFFT();

#include "SSD1306.h"  // https://github.com/squix78/esp8266-oled-ssd1306
SSD1306 display(0x3c, D2,D1);  // 0.96" OLED display object definition (address, SDA, SCL) Connect OLED SDA , SCL pins to ESP SDA, SCL pins
//샘플링
#define SAMPLES 256              //Must be a power of 2
#define SAMPLING_FREQUENCY 10000 //Hz, must be 10000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT.
#define amplitude 50
unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[] = {0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;

void setup() {
  Serial.begin(9600);
  Serial.print("\n");
  Serial.println("CLEARDATA");//시리얼 데이터를 엑셀에 저장하도록 초기화
  Serial.println("LABEL,Time,FFT");//엑셀 라벨들
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
      //Serial.println(i);
    }
    for (byte band = 0; band <= 6; band++) display.drawHorizontalLine(18*band,64-peak[band],14);
  }
  if (millis()%4 == 0) {for (byte band = 0; band <= 6; band++) {if (peak[band] > 0) peak[band] -= 1;}} // Decay the peak
  display.display();
}

int displayBand(int band, int dsize){
  int dmax = 50;
  if (dsize > dmax) {Serial.print("DATA,TIME,");}//엑셀에 데이터가 저장되도록 한 것 Serial.println(dsize);// 시리얼 표시 가능하게 한 것
  for (int s = 0; s <= dsize; s=s+2){display.drawHorizontalLine(18*band,64-s, 14); }
  if (dsize > peak[band]) {peak[band] = dsize; Serial.print("DATA,TIME,");}//엑셀에 데이터가 저장되도록 한 것 Serial.println(peak[band]);// 시리얼 표시 가능하게 한 것
}
```
경종 소리를 수집하여 엑셀 파일에 저장하고 평균값을 구하는 코드이다.

### complete_sound_message
```C
//fft 변환 라이브러리 없을 시 다운 -> https://github.com/kosme/arduinoFFT
#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT();

#include "SSD1306.h" 
SSD1306 display(0x3c, D2,D1);  // 0.96" OLED display object definition (address, SDA, SCL) Connect OLED SDA , SCL pins to ESP SDA, SCL pins
//샘플링 정의
#define SAMPLES 256              //Must be a power of 2
#define SAMPLING_FREQUENCY 10000 //Hz, must be 10000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT.
#define amplitude 50
unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[] = {0,0,0,0,0,0,0};
int count=0;
int mes=0;
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;
//------------------버튼, 데이터베이스, 문자전송-------------------------------
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ESP8266HTTPClient.h>
// 버튼 설정 
const int button1 = 14; // 출근버튼
int switch1 = 0;
const int togle = 13; //토글스위치, (0,1)A조 B조 데이터베이스 설정

// 와이파이 설정 
const char* ssid = "우옹";
const char* password = "12340000";

// lcd 설정
LiquidCrystal_I2C lcd(0x27,16,2);

void setup () {
  pinMode(button1, INPUT_PULLUP);//데이터베이스 전송버튼
  
  Wire.begin(5,4);
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  //display 설정
  display.init();
  display.setFont(ArialMT_Plain_10);
  display.flipScreenVertically(); // Adjust to suit or remove
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting..");
    
  }
  Serial.println("연결되었습니다.");
  lcd.init(); // LCD 설정
  lcd.clear();      // LCD를 모두 지운다.
  lcd.backlight();  // 백라이트를 켠다. 
// Arduino LCD, Welcome 표시 
  lcd.setCursor(0,0);
  lcd.print("Arduino LCD");
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print("Welcome");
  delay(1000);  
  
}
 
void loop() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("waiting signal");
  //-------------display 및 fft------------
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
  delay(1000);

//------------------------0 일때 php 전송------------------------
 if(digitalRead(button1) == LOW && digitalRead(togle) == LOW){
  Serial.println("0 버튼 눌림");
  
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;  //Declare an object of class HTTPClient
    //와이파이가 연결되어있으면 http 정의하고, 파일을 실행시킨다. GET으로 받아와서 실행시킨다.
    //php에 적혀있는 코드에 의해서 데이터베이스로 자동 입력된다.
    http.begin("http://www.all-tafp.org/apartA.php");  //Specify request destination
    int httpCode = http.GET();                                 //Send the request
 
    //lcd 출력 부분, 데이터베이스 전송 완료.
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("completed");
    delay(1500);  
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("waiting signal");
    Serial.println("데이터베이스 전송 완료");
    http.end();   //Close connection
    
   }
 }
//------------------------1 일때 php 전송------------------------
  if(digitalRead(button1) == LOW && digitalRead(togle) == HIGH){
  Serial.println("1 버튼 눌림");
  
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
      HTTPClient http;  //Declare an object of class HTTPClient
      //와이파이가 연결되어있으면 http 정의하고, 파일을 실행시킨다. GET으로 받아와서 실행시킨다.
      //php에 적혀있는 코드에 의해서 데이터베이스로 자동 입력된다.
     http.begin("http://www.all-tafp.org/apartB.php");  //Specify request destination
     int httpCode = http.GET();                                  //Send the request
 
     //lcd 출력 부분, 데이터베이스 전송 완료 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("completed");
      delay(1500);  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("waiting signal");
      Serial.println("데이터베이스 전송 완료");
      http.end();   //Close connection
   
     }
  }
  
}

//아날로그 신호를 디지털 신호로 바꾸어 display 화면에 띄우기 위한 함수
int displayBand(int band, int dsize){
  if(mes==1){mes=0;}
  int dmax = 50;  
  int dsizeprint = dsize;
  //display 화면에 띄우기 위한 제한 만약 쓸 경우 추가
  if (dsize > dmax) {dsize=dmax; }
  for (int s = 0; s <= dsize; s=s+2){display.drawHorizontalLine(18*band,64-s, 14); }
  if (dsize > peak[band]) {
    peak[band] = dsizeprint;
    //디지털 신호를 경종 신호에 맞춰서 비교 
    if(peak[band] == 26){count++;}
    if(28<= peak[band] && peak[band] <=29){count++;}
    if(32<= peak[band] && peak[band] <=33){count++;}
    if(40<= peak[band] && peak[band] <=41){count++;}
    }
    //경종 소리와 비슷할 경우 출력
    if(10<count && count<12){    
      if(count==11){count=0; mes=1;
      Serial.println("90% 일치합니다.");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("90percent match");
      delay(1500);
      lcd.clear();
      }
      //---------------------------- 문자 전송 툴
      //문자 입력시에 띄어쓰기는 %20으로 작성해야함. 아니면 작동X
      //Ex) 문자%20테스트입니다. -> 문자 테스트입니다. 
      String message = "http://www.all-tafp.org/EmmaSMS_PHP_UTF-8_JSON/example.php?sms_msg=화재발생!%20화재발생!.%20지금%20즉시%20관리사무소로%20와주시기바랍니다.&sms_to=010-8338-2385&sms_from=010-8338-2385&sms_date=";
      String alarm = "";
      if(mes == 1){
        Serial.println("메세지 보냄");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("send mes");
        delay(3000);
        lcd.clear();
        
        
        if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
          HTTPClient http;  //Declare an object of class HTTPClient
      
          http.begin(message);  //Specify request destination
          int httpCode = http.GET(); 
          //밑에는 http 문서 그대로 시리얼에 출력해서 확인용.
      /*if (httpCode > 0) { //Check the returning code
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);             //Print the response payload  */
          Serial.println("코드 실행");
          for(int i=40; i>0; i--){
          lcd.setCursor(0,0);
          lcd.print("waiting signal");
          lcd.setCursor(0,1);
          lcd.print(i);
          delay(1000);
          lcd.clear();
        }
           
     } 
  }
  }    
  if (peak[band]>0) {peak[band]=0;} // 소리를 세밀하게 측정하기 위해 초기화
}
```
경종 소리 발생시 소리를 입력받아 평균값과 비슷할 경우 경종 소리로 인식하고 관리자에게 화재경보 문자를 발송하는 코드이다.


## Arduino Serial
![Serial](https://user-images.githubusercontent.com/58980007/146053767-efbb1361-e63f-4fa3-9512-776fcf9cce0f.PNG)

마이크 모듈을 통해 입력받은 소릿값을 실시간으로 시리얼 모니터를 통해 수집한 자료이다.


## Excel Data
![excel 1](https://user-images.githubusercontent.com/58980007/146053816-96b8be73-7725-4053-8997-8a0b13d8b2d0.PNG)

입력받은 소릿값을 실시간으로 엑셀에 저장한 자료이다.

![excel 2](https://user-images.githubusercontent.com/58980007/146053822-10031501-feaa-484d-9c53-9b071cb178ba.PNG)

1, 2차에 수집한 데이터를 그래프로 나타낸 자료이다.

![excel 3](https://user-images.githubusercontent.com/58980007/146053823-254be5d0-20eb-4d76-86b9-17a2a823f485.PNG)

3, 4차에 수집한 데이터를 그래프로 나타낸 자료이다.

![excel 4](https://user-images.githubusercontent.com/58980007/146053824-0582caf8-e331-4c95-b52b-86dbeb9bf5fb.PNG)

5, 6차에 수집한 데이터를 그래프로 나타낸 자료이다.


## 완성사진
![System](https://user-images.githubusercontent.com/58980007/146052910-04136caa-e3bf-4404-8308-4c1cd28a7fd2.PNG)
