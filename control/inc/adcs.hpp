#ifndef __ADCS_H__
#define __ADCS_H__

#include "i2c.hpp"
#include "logs.hpp"

class adc_handler :
    virtual private Log,
    private I2C
{
public:
    static adc_handler &get_instance(void);
    int get_adcs_amount(void) { return ADCS_NB; }
    unsigned short get_adc_values(unsigned char);

private:
    static const int ADCS_NB = 4;

    adc_handler(): Log(false, true, true, true), I2C("/dev/i2c-1", 0x48) {}   
    void trigger_selected_adc(unsigned char);
};

#endif

