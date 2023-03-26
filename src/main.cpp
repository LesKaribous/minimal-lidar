#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2lib.h>
#include "LD06.h"

LD06 lidar;

const int LED_PIN = 4, LED_COUNT = 35 ;
const int MAX_LED_WIDHT = 5 ;
const int SDA_PIN = 18, SCL_PIN = 19 ;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2,SCL_PIN,SDA_PIN);

void colorWipe(uint32_t color, int wait);
void testOled();
void cleanStrip();
void circularTestStrip();
void randomTestStrip();
void drawAngle(int orientation, int width);
void testLidar();

void setup() {
  //u8g2.begin();
  Serial.begin(115200);
  Serial.println("Hello !");
  // Begin the serial transmission with the lidar
  lidar.Init();
  strip.begin();
  strip.show();
  strip.setBrightness(5);
  colorWipe(strip.Color(0,50,0),30);
  // Clean and start the main program
  colorWipe(strip.Color(0,0,0), 30);
  //testOled();
  //circularTestStrip();
}

void loop() {
  //randomTestStrip();
  testLidar();
}

void circularTestStrip()
{
  for(int j = 1000;j>0;j-=200)
  {
    for(int i = 0;i<=359;i++)
    {
      drawAngle(i,j);
      cleanStrip();
      delay(1);
    }
  }
  for(int j = 0;j<=1000;j+=200)
  {
    for(int i = 0;i<=359;i++)
    {
      drawAngle(i,j);
      cleanStrip();
      delay(1);
    }
  }
}

void randomTestStrip(){
  byte randomOrientation = random(0,359);
  int randomWidth = random (0,1000);
  drawAngle(randomOrientation, randomWidth);
  delay(1000);
  cleanStrip();
}

/*
void testOled(){
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 20);
    u8g2.print(F("Hello World!"));
  } while ( u8g2.nextPage() );
  delay(1000);
}
*/

void cleanStrip(){
  for(int i=0; i<strip.numPixels(); i++) strip.setPixelColor(i, 0);  
  strip.show();
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void drawAngle(int orientation, int width)
{
  //  Orientation doit être inférieure à 360°
  byte ledCenter  = map(orientation,0,359,0,LED_COUNT);
  byte ledWidth   = map(width,0,1000,MAX_LED_WIDHT,0);

  byte widthColor = map(width,0,1000,255,0);
  uint32_t color = strip.Color(widthColor,255-widthColor,0); // Modifie la couleur en fonction de la distance

  // Affiche la led de centre
  strip.setPixelColor(ledCenter, strip.Color(0,   0,   255));

  if (ledWidth != 0)
  {
    for(int i=1; i<=ledWidth; i++)
    {
      // Traite le décalage des leds par rapport au centre
      int ledPos = ledCenter + i;
      int ledNeg = ledCenter - i;

      // Gestion des valeurs en dehors des limites du ring de leds
      if ( ledPos > (LED_COUNT-1)) ledPos = ledPos - LED_COUNT ;
      if ( ledNeg < 0 ) ledNeg = ledNeg + LED_COUNT ;

      // Allume les leds 
      strip.setPixelColor(ledPos, color);
      strip.setPixelColor(ledNeg, color);
    }
  }
  strip.show();
}

void testLidar(){
  //Serial.println("data start");
  lidar.read_lidar_data();
  /*
  Serial.printf("start_bytet:%d, data_length:%d, Speed:%f, FSA:%f, LSA:%f, time_stamp:%d, CS:%d",
                lidar.start_byte, lidar.data_length, lidar.Speed, lidar.FSA, lidar.LSA, lidar.time_stamp, lidar.CS);
  Serial.println();
  */
  for (int i = 0; i < lidar.data_length; i++) 
  {
    if (lidar.distances[i] < 65535.0)
    {
      Serial.print(">angle:");
      Serial.println(lidar.angles[i]);
      Serial.print(">distance:");
      Serial.println(lidar.distances[i]);
    }
    
    if (lidar.distances[i] > 300.0 && lidar.distances[i] < 1000){
      drawAngle(lidar.angles[i], 999);
    }
  }
}