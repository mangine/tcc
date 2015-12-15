/**************************************************************************/
/*!
@file     MQ135.h
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3
First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.
@section  HISTORY
v1.0 - First release
*/
/**************************************************************************/

#ifndef MQ135_H_
#define MQ135_H_

#include <mraa.h>
#include <math.h>

class MQ135 {
private:
	/// The load resistance on the board
	const float RLOAD = 10.0f;

	/// Calibration resistance at atmospheric CO2 level
	const float RZERO = 76.63f;
	/// Parameters for calculating ppm of CO2 from sensor resistance
	const double PARA = 116.6020682f;
	const double PARB = 2.769034857f;

	/// Parameters to model temperature and humidity dependence
	const double CORA = 0.00035f;
	const double CORB = 0.02718f;
	const double CORC = 1.39538f;
	const double CORD = 0.0018f;

	/// Atmospheric CO2 level for calibration purposes
	const float ATMOCO2 = 397.13f;


	mraa_aio_context _pin;
	mraa_gpio_context _flag;
public:
	MQ135(uint8_t pin, bool digital=true);
	float getCorrectionFactor(float t, float h);
	float getResistance();
	float getCorrectedResistance(float t, float h);
	float getPPM();
	float getCorrectedPPM(float t, float h);
	float getRZero();
	float getCorrectedRZero(float t, float h);
    int ReadFlag();

};


#endif
