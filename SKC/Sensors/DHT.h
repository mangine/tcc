/*
 * DHT.h
 *
 *  Created on: Nov 12, 2015
 *      Author: Matheus
 */

#ifndef DHT_H_
#define DHT_H_

#define MAXTIMINGS 85

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

#include <math.h>
#include <sys/time.h>
#include <stdint.h>
#include <mraa.h>
#include <time.h>

class DHT {
 private:
  uint8_t data[6];
  mraa_gpio_context _inpin, _outpin;
  uint8_t _type, _count;
  unsigned long _lastreadtime;
  timespec _lastread;


  int bitsToByte(int bits[]);

 public:
  DHT(uint8_t inPin, uint8_t outPin,uint8_t type, uint8_t count=6);
  float readTemperature(bool S=false);
  float convertCtoF(float);
  float convertFtoC(float);
  float computeHeatIndex(bool f=false);
  float readHumidity(void);
  bool read(void);


};



#endif /* DHT_H_ */
