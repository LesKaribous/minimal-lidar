#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2lib.h>
#include "ld06.h"

LD06 lidar;

const int LED_PIN = 4, LED_COUNT = 35 ;
const int MAX_LED_WIDHT = 5 ;
const int SDA_PIN = 18, SCL_PIN = 19 ;

long lastClean = 0;

int heading = 0; // Angle du regard
int range   = 0; // Distance du regard
volatile bool newData = false;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2,SCL_PIN,SDA_PIN);

void colorWipe(uint32_t color, int wait);
void testOled();
void cleanStrip();
void circularTestStrip();
void randomTestStrip();
void drawAngle(int orientation, int width);
void testLidar();
void ringLidar(int lHeading, int lRange, int lSector);
void checkNeworder();
bool isHeadingInSector(int cAngle, int cHeading, int cSector);

void setup() {
  //u8g2.begin();
  Serial.begin(115200);
  Serial.println("Hello !");
  // Begin the serial transmission with the lidar
  lidar.init(6);
  strip.begin();
  strip.show();
  strip.setBrightness(50);
  colorWipe(strip.Color(0,50,0),30);
  // Clean and start the main program
  colorWipe(strip.Color(0,0,0), 30);
  //testOled();
  //circularTestStrip();
}

void loop() {
  //randomTestStrip();
  //testLidar();
  checkNeworder();
  ringLidar(heading,range,30);
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
  lastClean = millis();
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
  byte ledCenter  = map(orientation,0,359,LED_COUNT,0);
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

void testLidar()
{
  // Utilise TelePlot pour tracer la map de la pièce (test du lidar)
  if(lidar.readFullScan())
  {
    lidar.teleplotPrintScan();
  }
}

void ringLidar(int lHeading, int lRange, int lSector)
{
  // Trace les points de detection sur le ring de led en fonction de la distance donnée
  if(lidar.readFullScan())
  {
    for (uint16_t i = 0; i < lidar.scan.size(); i++) 
    {
      int objDistance = int(lidar.scan[i].distance);
      int objAngle = int(lidar.scan[i].angle);

      if (objDistance > 200 && objDistance < lRange && isHeadingInSector(objAngle,lHeading,lSector))
      {
        int ledCenter  = map(objAngle,0,359,0,LED_COUNT);
        int ledRed    = map(objDistance,200,1000,255,0);
        int ledGreen  = map(objDistance,200,1000,0,255);

        /*
        Serial.print("Detection en ");
        Serial.print(objAngle);
        Serial.print("° à ");
        Serial.print(objDistance);
        Serial.println(" mm");
        */

        strip.setPixelColor(ledCenter, strip.Color(ledRed,   ledGreen,   10));
      }
    }
    strip.show();
  }
  // Supprime les points tous les 500ms
  if(millis()-lastClean >= 500) cleanStrip();
}

bool isHeadingInSector(int cAngle, int cHeading, int cSector) {
    int minHeading = cHeading - cSector;
    int maxHeading = cHeading + cSector;
    
    // Vérifie si cAngle est dans l'intervalle angulaire [minHeading, maxHeading]
    if ((cAngle >= minHeading && cAngle <= maxHeading) || 
        (cAngle + 360 >= minHeading && cAngle + 360 <= maxHeading) ||
        (cAngle - 360 >= minHeading && cAngle - 360 <= maxHeading)) {
        return true;
    } else {
        return false;
    }
}

void checkNeworder(){
  if (newData) 
  {
    Serial.print("Look at: ");
    Serial.print(heading);
    Serial.print(", ");
    Serial.println(range);
    newData = false; // Réinitialise la variable newData
  }
}

void serialEvent() {
  Serial.println("Reading...");
  // Lit les données jusqu'à la fin de la trame
  String inputString = Serial.readStringUntil('\n');
  if (inputString.charAt(0) == 'G') {
    // Extraction des données à partir de la trame
    int commaIndex = inputString.indexOf(',');
    int xIndex = 1;
    int yIndex = commaIndex + 1;
    if (commaIndex != -1 && inputString.charAt(inputString.length() - 1) == ';') {
      // Conversion des données en entiers
      String xString = inputString.substring(xIndex, commaIndex);
      String yString = inputString.substring(yIndex, inputString.length() - 1);
      heading = xString.toInt();
      range   = yString.toInt();
      newData = true; // Active le flag newData lorsque des données sont disponibles
    } else {
      Serial.println("Invalid data format");
    }
  } else {
    Serial.println("Invalid data format");
  }
}