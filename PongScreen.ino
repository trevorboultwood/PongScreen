#include <FastLED.h>
#include <SPI.h>
#include <SD.h>
#include <IRremote.h>

#define NUM_LEDS 286
#define DATA_PIN 2
#define CMD_NEW_DATA 1
#define BAUD_RATE 500000  //if using Glediator via serial
#define SD_CS 10 //check for mega.
#define IR_recv 12//check for mega.

IRrecv irrecv(IR_recv);
decode_results IR_results;

File fxdata;
CRGB leds[NUM_LEDS];

int dimmingValue = 0; //can be 0-9 (default '0' dim)
int speedValue = 50; //can be anything from 0-100 in 10ms increments
unsigned long sizeOfFile;

void setup()
{
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS); //se doc for different LED strips
  Serial.begin(115200); //For Debugging
  for(int y = 0 ; y < NUM_LEDS ; y++) 
  {
    leds[y] = CRGB::Black; // set all leds to black during setup
  }
  FastLED.show();

  pinMode(SD_CS, OUTPUT); // CS/SS pin as output for SD library to work.
  digitalWrite(SD_CS, HIGH); // workaround for sdcard failed error...

  if (!SD.begin(4))
  {
    Serial.println("sdcard initialization failed!");
    return;
  }
  Serial.println("sdcard initialization done.");
  
  // test file open
  fxdata = SD.open("myanim.dat");  // read only
  if (fxdata)
  {
    Serial.println("file open ok");      
    fxdata.close();
  }
  Serial.println("Enabling IR Remote");
  irrecv.enableIRIn();
  Serial.println("Enabled IRin");



  
}
void IR_implement()
{
  if (irrecv.decode(&IR_results))
  {
    Serial.println(IR_results.value, HEX);
    irrecv.resume(); // Receive the next value
    //Action result
    switch(IR_results.value)
    {
      case 111: //"incresedim":
      if(dimmingValue + 1 > 10)
      {
        dimmingValue = 10;
      }
      else
      {
        dimmingValue = dimmingValue + 1;
      }
      case 222://"dim decrease":
      if(dimmingValue - 1 < 0)
      {
        dimmingValue = 0;
      }
      else
      {
        dimmingValue = dimmingValue - 1;
      }
      case 333://"speed up":
      if(speedValue - 10 < 0)
      {
        speedValue = 0;
      }
      else
      {
        speedValue = speedValue - 10;
      }
      case 444://"speed down":
      if(speedValue + 10 > 100)
      {
        speedValue = 100;
      }
      else
      {
        speedValue = speedValue + 10;
      }
      case 555://"next":
      fxdata.seek(sizeOfFile);
      
      case 666://"pause":
      
      break;   
    } 
  }
}
void loop()
{
  //Variables

  // Decide FileNames etc
  //TryToRUN
  
  fxdata = SD.open("myanim.dat");  // read only
  sizeOfFile = fxdata.size();//Used for getting to end of file.
  
  if (fxdata)
    {
      Serial.println("file open ok");      
    }

  while (fxdata.available()) 
  {
    //Read IR Code
    if (irrecv.decode(&IR_results))
      {
        Serial.println(IR_results.value, HEX);
        irrecv.resume(); // Receive the next value
      }
    //IDEAS - Next, dim up, dim down, pause, OFF/ON,
    
    fxdata.readBytes((char*)leds, NUM_LEDS*3);    
    for(int i = 0; i < NUM_LEDS;i++)
      {
        int redValue = leds[i][1];
        int greenValue = leds[i][2];
        int blueValue = leds[i][3];
        redValue = map(redValue,0,255,0,250);//255 is hard to scale to reach 0
        greenValue = map(greenValue,0,255,0,250);
        blueValue = map(blueValue,0,255,0,250);
        
        if(redValue - (dimmingValue*25) < 0)
          {
            redValue = 0;
          }
        else
          {
            redValue = redValue - (dimmingValue*25);
          }
        if(greenValue - (dimmingValue*25) < 0)
          {
            greenValue = 0;
          }
        else
          {
            greenValue = greenValue - (dimmingValue*25);
          }
        if(blueValue - (dimmingValue*25) < 0)
          {
            blueValue = 0;
          }
        else
          {
            blueValue = blueValue - (dimmingValue*25);
          }
        
      }
    FastLED.show();
    delay(speedValue); // set the speed of the animation. 20 is appx ~ 500k bauds
  }
  
  // close the file in order to prevent hanging IO or similar throughout time
  fxdata.close();
}
