#include <hsl2rgb.h>

#include <Adafruit_NeoPixel.h>

#define PIN 6
#define LED_COUNT (60*3)

#define FLAME_WIDTH 20
#define FLAME_COUNT (LED_COUNT/FLAME_WIDTH)


static byte lightPowerMap[256] = {
   0,2,0,1,0,2,1,1,3,1,0,1,2,1,1,2,
   2,3,2,1,2,1,2,2,3,4,3,2,3,4,3,3,
   4,3,4,5,4,4,5,4,5,6,5,6,7,6,5,6,
   7,6,7,6,8,7,8,8,9,8,9,10,9,10,10,11,
   11,11,12,12,12,13,13,13,14,14,15,15,15,16,16,17,
   17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,
   25,25,26,26,27,28,28,29,29,30,31,31,32,32,33,34,
   34,35,36,37,37,38,39,39,40,41,42,43,43,44,45,46,
   47,47,48,49,50,51,52,53,54,54,55,56,57,58,59,60,
   61,62,63,64,65,66,67,68,70,71,72,73,74,75,76,77,
   79,80,81,82,83,85,86,87,88,90,91,92,94,95,96,98,
   99,100,102,103,105,106,108,109,110,112,113,115,116,118,120,121,
   123,124,126,128,129,131,132,134,136,138,139,141,143,145,146,148,
   150,152,154,155,157,159,161,163,165,167,169,171,173,175,177,179,
   181,183,185,187,189,191,193,196,198,200,202,204,207,209,211,214,
   216,218,220,223,225,228,230,232,235,237,240,242,245,247,250,252
};

// 0 index is center of flame
static byte hueMap[FLAME_WIDTH] = {
  128, 128, 123, 118, 113, 108, 103, 98, 93, 75, 
  50, 35, 30, 25, 20, 15, 10, 5, 0, 0
};

// 0 index is center of flame
static byte saturationMap[FLAME_WIDTH] = {
    64, 128, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

// 0 index is center of flame
static byte brightnessMap[FLAME_WIDTH] = {
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 128, 64, 32
};

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);


int8_t centerOffset[FLAME_COUNT];
uint8_t flameSize[FLAME_COUNT];

uint8_t dimmer = 255;

void setup() {
  Serial.begin(57600);
  
  pinMode(A0, OUTPUT);
  pinMode(A2, INPUT);
  pinMode(A4, OUTPUT);
  
  digitalWrite(A0, LOW);
  digitalWrite(A4, HIGH);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  for (uint8_t flame = 0; flame < FLAME_COUNT; ++flame) {
    flameSize[flame] = 255;
    centerOffset[flame] = 0;
  }
}

void loop() {
  long start = millis();
  uint8_t rgb[3];
  dimmer = analogRead(2) / 4;
  for (uint8_t flame = 0; flame < FLAME_COUNT; ++flame) {
    const int startPixel = flame * FLAME_WIDTH;

    uint8_t center = FLAME_WIDTH/2 + centerOffset[flame];
    for (uint8_t i = 0; i < FLAME_WIDTH; ++i) {
      byte index;
      if (i < center) {
        uint32_t temp = center;
        temp -= i;
        temp *= (FLAME_WIDTH-1);
        temp /= center;
        index = temp;
//        index = (byte)((int)(center-i) * (FLAME_WIDTH-1) / (center));
      } else if (i == center) {
        index = 0;
      } else {
        uint32_t temp = i;
        temp -= center;
        temp *= FLAME_WIDTH-1;
        temp /= (FLAME_WIDTH-center);
        index = temp;
//        index = (byte)((int)(i-center) * (FLAME_WIDTH-1) / (FLAME_WIDTH-center));
      }
      if (index < 0 || index >= 20) {
        Serial.println("Crap");
      } else {
        uint8_t hue = hueMap[index];
        uint8_t sat = saturationMap[index];
        uint8_t bri = brightnessMap[index];
        uint16_t lum = bri;
        lum = lum * ((uint16_t)dimmer+1) / 256;
        lum = lightPowerMap[lum];
        lum = lum * ((uint16_t)flameSize[flame]+1) / 256;
  
        hsl2rgb(hue, sat, lum, rgb);
        strip.setPixelColor(startPixel + i, Adafruit_NeoPixel::Color(rgb[0], rgb[1], rgb[2]));
      }
    }
    
    if (random(100) < 50) {
      centerOffset[flame] += random(5)-2;
      centerOffset[flame] = (byte)max(-9, min(8, centerOffset[flame]));
    }

    if (random(100) < 10) {
      flameSize[flame] += (int32_t)flameSize[flame] + rand(10, 20, 12, 1, 1);
      flameSize[flame] = (uint8_t)max(0, min(255, flameSize[flame]));
    }
    
  }
  strip.show();
  
  long end = start + (1000/60);
  if (millis() >= end) {
    Serial.println("too slow");
  }
  while (millis() < end);
}

int rand(int decRatio, int zeroRatio, int incRatio, int maxDec, int maxInc) {
  int totalRatio = decRatio + zeroRatio + incRatio;
  
  int ratio = random(totalRatio);
  if (ratio < decRatio) {
    return -(random(maxDec)+1);
  }
  ratio -= decRatio;
  if (ratio < incRatio) {
    return random(maxInc)+1;
  }
  return 0;
}
