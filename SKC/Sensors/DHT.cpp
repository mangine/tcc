#ifdef __MRAA__

/* DHT library

MIT license
written by Adafruit Industries

Modified by Dino Tinitigan (dino.tinitigan@intel.com to work on Intel Galileo boards
*/


#include "DHT.h"

DHT::DHT(uint8_t inPin, uint8_t outPin, uint8_t type, uint8_t count) {

	_inpin = mraa_gpio_init(inPin);
	_outpin = mraa_gpio_init(outPin);
	mraa_gpio_dir(_inpin,MRAA_GPIO_IN);
	mraa_gpio_dir(_outpin,MRAA_GPIO_OUT_HIGH);
	mraa_gpio_use_mmaped(_inpin,1);
	mraa_gpio_use_mmaped(_outpin,1);

	_type = type;
	_count = count;

	clock_gettime(CLOCK_MONOTONIC, & _lastread);
}



//boolean S == Scale.  True == Farenheit; False == Celcius
float DHT::readTemperature(bool S) {
  float f;

  if (read()) {
    switch (_type) {
    case DHT11:
      f = data[2];
      if(S)
        f = convertCtoF(f);

      return f;
    case DHT22:
    case DHT21:
      f = data[2] & 0x7F;
      f *= 256;
      f += data[3];
      f /= 10;
      if (data[2] & 0x80)
    f *= -1;
      if(S)
    f = convertCtoF(f);

      return f;
    }
  }
  return NAN;
}

float DHT::convertCtoF(float c) {
    return c * 9 / 5 + 32;
}
float DHT::convertFtoC(float f) {
    return (f-32)*5/9;
}

float DHT::readHumidity(void) {
  float f;
  if (read()) {
    switch (_type) {
    case DHT11:
      f = data[0];
      return f;
    case DHT22:
    case DHT21:
      f = data[0];
      f *= 256;
      f += data[1];
      f /= 10;
      return f;
    }
  }
  return NAN;
}

float DHT::computeHeatIndex(bool f) {
  // Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
  // Wikipedia: http://en.wikipedia.org/wiki/Heat_index
	float tempFahrenheit = readTemperature(true);
	float percentHumidity = readHumidity();
	float ret = -42.379 +
	           2.04901523 * tempFahrenheit +
	          10.14333127 * percentHumidity +
	          -0.22475541 * tempFahrenheit*percentHumidity +
	          -0.00683783 * pow(tempFahrenheit, 2) +
	          -0.05481717 * pow(percentHumidity, 2) +
	           0.00122874 * pow(tempFahrenheit, 2) * percentHumidity +
	           0.00085282 * tempFahrenheit*pow(percentHumidity, 2) +
	          -0.00000199 * pow(tempFahrenheit, 2) * pow(percentHumidity, 2);
	if(!f) return convertFtoC(ret);
	else return ret;
}


bool DHT::read(void) {
	timespec t1,t2,cur;
	int count;
	const int timeout = 150;

	int bitContainer[8];
	clock_gettime(CLOCK_MONOTONIC,&cur);
	// Check if sensor was read less than two seconds ago and return early
	// to use last reading.

	if (data[0]!=0 && (cur.tv_sec - _lastread.tv_sec < 2000)) {
	return true; // return last correct measurement
	}

	/*
	Serial.print("Currtime: "); Serial.print(currenttime);
	Serial.print(" Lasttime: "); Serial.print(_lastreadtime);
	*/
	clock_gettime(CLOCK_MONOTONIC,&_lastread);

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;


	// now pull it low for ~20 milliseconds
	////noInterrupts();
	mraa_gpio_write(_outpin,1); //garante que esta 1
	usleep(20000); // sleeps for 20ms
	mraa_gpio_write(_outpin,0);
	usleep(20000); //pelo menos 20ms
	mraa_gpio_write(_outpin,1);

	while(mraa_gpio_read(_inpin)==1 && count < timeout) count++; //valida que recebeu comando
	if(count>=timeout) return false;
	count = 0;

	while(mraa_gpio_read(_inpin)==0 && count < timeout) count++; //valida que esta em 0
	if(count>=timeout) return false;
	count = 0;

	while(mraa_gpio_read(_inpin)==1 && count < timeout) count++; //valida que subiu pra 1 e vai mandar dados
	if(count>=timeout) return false;
	count = 0;


	for(int bytes = 0; bytes < 5; bytes++)
	{
		for(int i = 0; i < 8; i++)
		{

			while(mraa_gpio_read(_inpin)==0) count++; //pre bit
			if(count >= timeout) return false;
			count = 0;

			clock_gettime(CLOCK_MONOTONIC,&t1); //sensor pre bit (50us)
			while(mraa_gpio_read(_inpin)==1) count++; //bit
			clock_gettime(CLOCK_MONOTONIC,&t2); //sensor pre bit
			if(count > timeout) return false;
			count = 0;

			if(t2.tv_nsec-t1.tv_nsec > 45000) bitContainer[i] = 1;
			else  bitContainer[i] = 0;

		}
		data[bytes] = bitsToByte(bitContainer);
	}

	///PODE INTERROMPER

    if((data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)))
	{
		return true;
	}


  return false;

}


int DHT::bitsToByte(int bits[])
{
	int data = 0;
	for(int i = 0; i < 8; i++)
	{
		if (bits[i])
		{
            data |= (int)(1 << (7 - i));
		}
	}
	return data;
}

#endif
