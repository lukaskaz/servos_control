#include <unistd.h>

#include "adcs.hpp"


void adc_handler::trigger_selected_adc(unsigned char mux)
{
    unsigned char reg = 0xC3 + (mux<<4);
    unsigned char data[] = { 0x01, reg, 0x83 };

    i2c_transfer(I2C::REQ_WRITE, data, sizeof(data));
    while(true) {
 	unsigned char res[2] = { 0x01 };
        i2c_transfer(I2C::REQ_READ, res, sizeof(res));
	if(res[0] & 0x80) {
            break;
	}
    }
}

unsigned short adc_handler::get_adc_values(unsigned char adc_nb)
{
    unsigned short ret = 0;		
    unsigned char data[2] = {0};
	    
    trigger_selected_adc(adc_nb);
    i2c_transfer(I2C::REQ_READ, data, sizeof(data));
    	
    ret = (unsigned short)(data[1]|(data[0]<<8));
    ret = ret/10;

    return ret;
}

adc_handler &adc_handler::get_instance(void)
{
    static adc_handler adcHdlr;
    return adcHdlr;
}




