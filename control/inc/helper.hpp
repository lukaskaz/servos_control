#ifndef __HELPER_H__
#define __HELPER_H__

#include "types.hpp"
#include "gpio.hpp"
#include "logs.hpp"

#define DEV_MEM     "/dev/mem"


class gpio_handler : 
    virtual private Log
{
public:
    gpio_handler(): Log(false, true, true, true)
                    { init();    }
    ~gpio_handler() { destroy(); } 

protected:
    void set_relay(uint8_t);
    void switch_relay(void);
    void set_buzzer(uint8_t);    

    uint8_t get_relay_state(void);
    uint8_t get_emerg_state(void);

private:
    static const uint32_t platform = PERIPH_BASE_RPI2;
    gpio_t* gpio;

    int8_t init(void);
    int8_t destroy(void);

    void *mapmem(uint32_t base, uint32_t size, const char *mem_dev);
    void *unmapmem(void *addr, uint32_t size);
};



#endif

