#ifdef __MRAA__
#ifndef POWERPIN_H_INCLUDED
#define POWERPIN_H_INCLUDED

#include "mraa.h"

class PowerPin{
private:
    mraa_gpio_context p = NULL;

public:
    PowerPin(uint8_t pin){
        p = mraa_gpio_init(pin);
        mraa_gpio_dir(p, MRAA_GPIO_OUT_LOW);
    }

    ~PowerPin(){
        mraa_gpio_close(p);
    }

    void PowerUp(){
        if(p==NULL) return;
        mraa_gpio_write(p,1);
    }

    void PowerDown(){
        if(p==NULL) return;
        mraa_gpio_write(p,0);
    }
};

#endif // POWERPIN_H_INCLUDED
#endif
