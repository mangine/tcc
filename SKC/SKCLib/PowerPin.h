#ifdef __MRAA__
#ifndef POWERPIN_H_INCLUDED
#define POWERPIN_H_INCLUDED

class PowerPin{
    mraa_gpio_context p;

    PowerPin(uint8_t pin){
        p = mraa_gpio_init(pin);
        mraa_gpio_dir(p, MRAA_GPIO_OUT_LOW);
    }

    ~PowerPin(){
        mraa_gpio_close(p);
    }

    PowerUp(){
        mraa_gpio_write(mraa_gpio_context,1);
    }

    PowerDown(){
        mraa_gpio_write(mraa_gpio_context,0);
    }
}

#endif // POWERPIN_H_INCLUDED
#endif
