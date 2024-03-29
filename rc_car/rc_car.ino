/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include <Servo.h> 
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
 Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
/*
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
*/


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];


/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
//Motor Pins
//EGEN 310 CODE
#define Bin1 6
#define Bin2 5
#define Ain1 3
#define Ain2 5
// Declare the Servo pin 
int servoPin = 2;
// Initial Speed
int speed = 255;
// Initial Servo Center Angle
int center = 90;
// Fine Tune Variables
bool fineTune = false;
int fineTuneStep = 1;
int regStep = 5;
int currentStep = regStep;

Servo Servo1;

void setup(void)
{
  pinMode(Bin1, OUTPUT);
  pinMode(Bin2, OUTPUT);
  pinMode(Ain1, OUTPUT);
  pinMode(Ain2, OUTPUT);
  Servo1.attach(servoPin); 
  pinMode(LED_BUILTIN, OUTPUT);
  
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit App Controller Example"));
  Serial.println(F("-----------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }


  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;
  
  // Buttons
  //EGEN 310 CODE
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';

    //Button 1 speed control up
    if(buttnum == 1){
      if (pressed) {
        if(speed < 245){
          speed = speed+10;
        }
        else if(speed < 255){
          speed = speed+1;
        }
      }
    }
    //Button 2 speed control down
    else if(buttnum == 2){
      if (pressed) {
        if(speed > 10){
          speed = speed-10;
        }
        else if(speed > 0){
          speed = speed-1;
        }
      }
    }
    //Steering buttons 7 and 8
    else if(buttnum == 7){
      while (pressed  and Servo1.read() < 180) {
          int current_pos;
          current_pos = Servo1.read();
          Servo1.write(current_pos + currentStep);

        uint8_t len = readPacket(&ble, 20);
        if (packetbuffer[1] == 'B') {
          uint8_t buttnum = packetbuffer[2] - '0';
          if(buttnum == 7){
            pressed = packetbuffer[3] - '0';
          }
        }
      } 
    }
    else if(buttnum == 8){
     while (pressed and Servo1.read() > 0) {
        int current_pos;
        current_pos = Servo1.read();
        Servo1.write(current_pos - currentStep);
        
      uint8_t len = readPacket(&ble, 20);
        if (packetbuffer[1] == 'B') {
          uint8_t buttnum = packetbuffer[2] - '0';
          if(buttnum == 8){
            pressed = packetbuffer[3] - '0';
          }
        }
     }
    }
    //Center button 3
    else if(buttnum == 3){
     if (pressed) {
        Servo1.write(center);
     }
    }

    //Fine tune steering button 3
    else if(buttnum == 4){
     if (pressed) {
        if (fineTune) {
          int new_center;
          new_center = Servo1.read();
          center = new_center;
          currentStep = regStep;
          fineTune = false;
        }
        else {
          currentStep = fineTuneStep;
          fineTune = true;
        }
     }
    }
    //Drive control buttons 5 and 6
    else if(buttnum == 5){
      if (pressed) {
        analogWrite(Bin1, speed);
        analogWrite(Ain1, speed);
      } else {
        analogWrite(Bin1, 0);
        analogWrite(Ain1, 0);
      }
    }
    else if(buttnum == 6){
      if (pressed) {
        analogWrite(Bin2, speed);
        analogWrite(Ain2, speed);
        
      } else {
        analogWrite(Bin2,0);
        analogWrite(Ain2,0);
      }
    }
  }
}
