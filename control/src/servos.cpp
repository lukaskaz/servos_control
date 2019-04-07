#include <iostream>
#include <unistd.h>

#include <string>
#include <cstring>
#include <thread>

#include "rgb_if.hpp"
#include "helper.hpp"
#include "adcs.hpp"
#include "servos.hpp"


#define SERVOS_I2C_BUS          "/dev/i2c-1"

#define CONFIG_REG_DEFAULT	0x1111

#define ADC_DEADBAND_LOW        1200
#define ADC_DEADBAND_HIGH       1400
#define IS_ADC_DEADBAND(adc)\
                ((adc) > ADC_DEADBAND_LOW && (adc) < ADC_DEADBAND_HIGH)

#define SERVO_POS(servo)\
                ( servo.end_tilt * (servo.pos - servo.limit[LIMIT_LOW])/ \
		(servo.limit[LIMIT_HIGH] - servo.limit[LIMIT_LOW]) )	

#define SERVOS_NB SERVO_LAST
#define SERVOS_PER_ADC 2


#define SERVO_1_CONFIG \
"SERVO_1",      /* name */ \
0x0206,         /* init position */ \
{ 91,  528 },   /* extreme limits */ \
DIR_NORMAL,     /* base direction */ \
0, 270,         /* start point and end point reference tilt (degree) */ \
SERVO_NONE, 0,  /* servo conflicts */ \
&servos_handler::servo_1_pos /* servo raw to tilt calc handler */

#define SERVO_2_CONFIG \
"SERVO_2",      /* name */ \
0x012F,         /* init position */ \
{ 162,  455 },  /* extreme limits */ \
DIR_NORMAL,     /* base direction */ \
0, 170,         /* start point and end point reference tilt (degree) */ \
SERVO_3, 90,    /* servo conflicts */ \
&servos_handler::servo_2_pos /* servo raw to tilt calc handler */

#define SERVO_3_CONFIG \
"SERVO_3",      /* name */ \
0x014F,         /* init position */ \
{ 150,  480 },  /* extreme limits */ \
DIR_REVERSED,   /* base direction */ \
0, 170,         /* start point and end point reference tilt (degree) */ \
SERVO_2, 90,    /* servo conflicts */ \
&servos_handler::servo_3_pos /* servo raw to tilt calc handler */

#define SERVO_4_CONFIG \
"SERVO_4",      /* name */ \
0x007E,         /* init position */ \
{ 126,  501 },  /* extreme limits */ \
DIR_NORMAL,     /* base direction */ \
0, 180,         /* start point and end point reference tilt (degree) */ \
SERVO_NONE, 0,  /* servo conflicts */ \
&servos_handler::servo_4_pos /* servo raw to tilt calc handler */

    
std::mutex servos_handler::servo_mtx;

void servos_handler::init_data(void)
{
    servos.reserve(SERVOS_NB);

    servos.push_back(servo_t(SERVO_1_CONFIG));
    servos.push_back(servo_t(SERVO_2_CONFIG));
    servos.push_back(servo_t(SERVO_3_CONFIG));
    servos.push_back(servo_t(SERVO_4_CONFIG));
}

bool servos_handler::is_emerg_active(void)
{
    if(get_emerg_state() != 0) {
        log(LOG_INFO, __func__, "Reset requested, aborting servo manual control!");
	init_device();
	return true;
    }

    return false;
}

void servos_handler::run(void)
{
    static const uint8_t delay_ms = 10;

    while(1) {
        std::unique_lock<std::mutex> lock(servo_mtx);
    
        reinit_device();                                            
        if(is_emerg_active() == false) {
            lock.unlock();
            usleep(delay_ms*1000);
        }
        else {
            lock.unlock();
            break;
        }
    }
}

void servos_handler::energise_servos(void)
{
    if(get_relay_state() != 0) {
        log(LOG_DEBUG, __func__, "Trying to power up servos");
        set_relay(1);

	sleep(1);
	log(LOG_DEBUG, __func__, "Servos should be powered now");
    }
}

void servos_handler::disengage_servos(void)
{
    if(get_relay_state() == 0) {
        log(LOG_DEBUG, __func__, "Trying to power off servos");
	sleep(1);
	
        set_relay(0);
	log(LOG_DEBUG, __func__, "Servos should be off now");
    }
}

void servos_handler::init_servos_driver(void)
{
    set_servos_config(0x00, 0x10);
    set_servos_config(0xFE, 0x7A);
    set_servos_config(0x00, 0xA0);
}

void servos_handler::set_servos_init_position(void)
{
    for(auto s = servos.begin(); s != servos.end(); s++) {
        s->pos = s->init_pos;
        s->tilt = (this->*(s->tilt_hdlr))(s, s->pos);
    }

    write_servos_all_pos();
}

void servos_handler::init(void)
{
    init_data();
    init_device(); 
    
    std::thread suppThread(&servos_handler::trackAndUpdateThread, this);
    suppThread.detach();
}

void servos_handler::init_device(void)
{
    energise_servos();
    init_servos_driver();    
    set_servos_init_position();
}

int8_t servos_handler::read_servos_adcs(void)
{
    int8_t ret = (-1);
    adc_handler &adc = adc_handler::get_instance(); 

    for(auto s=servos.begin(); s != servos.end(); s++) {
        uint16_t val = adc.get_adc_values(std::distance(servos.begin(), s));

        if(IS_ADC_DEADBAND(val)) {
            s->adc = DATA_NOT_AVAIL;
        }
        else {
            s->adc = val;
            ret = 0;
        }    }
    return ret;
}

int8_t servos_handler::read_servos_pos(void)
{
    if(read_servos_adcs() == 0) {
        static const uint8_t regs[SERVOS_NB] = { 0x08, 0x0c, 0x10, 0x14 };
        int8_t i = 0;

        for(auto s=servos.begin(); s != servos.end(); s++, i++) {
            uint8_t data[] = { regs[i], 0x00 };
            if(s->adc != DATA_NOT_AVAIL) { 
                i2c_transfer(REQ_READ, data, sizeof(data));
                s->pos = (data[1]<<8)|(data[0]);
            }
        }

        return 0;
    }

    return (-1);                                              
}

void servos_handler::read_servos_config(uint8_t* config, size_t size)
{
	uint8_t data[size] = { 0x00, };

        i2c_transfer(REQ_READ, data, size);
        std::memcpy(config, data, size);
}

void servos_handler::set_servos_config(uint8_t reg, uint8_t value)
{
    uint8_t config[] = { reg, value };
    i2c_transfer(REQ_WRITE, config, sizeof(config));
}

void servos_handler::write_servos_pos(void)
{
    static const uint8_t regs[SERVOS_NB] = { 0x08, 0x0c, 0x10, 0x14 };
    uint8_t i = 0;        

    for(auto s=servos.begin(); s != servos.end(); s++, i++) {
        if(s->adc != DATA_NOT_AVAIL) {
            uint8_t data[] = { regs[i], (uint8_t)(s->pos), (uint8_t)(s->pos>>8) };
            i2c_transfer(REQ_WRITE, data, sizeof(data));
        }
    }
}

void servos_handler::write_servos_all_pos(void)
{
    uint8_t data[] = 
    {
        0x06, 
        0, 0, (uint8_t)(servos[0].pos), (uint8_t)(servos[0].pos>>8),
        0, 0, (uint8_t)(servos[1].pos), (uint8_t)(servos[1].pos>>8),
        0, 0, (uint8_t)(servos[2].pos), (uint8_t)(servos[2].pos>>8),
        0, 0, (uint8_t)(servos[3].pos), (uint8_t)(servos[3].pos>>8),
    };

    i2c_transfer(REQ_WRITE, data, sizeof(data));
}


#define SERVO_REDUCE_TO_LIMIT_LOW(ser,pos)\
            (((pos) < ser.limit[LIMIT_LOW]) ? ser.limit[LIMIT_LOW] : (pos))

#define SERVO_REDUCE_TO_LIMIT_HIGH(ser,pos)\
            (((pos) > ser.limit[LIMIT_HIGH]) ? ser.limit[LIMIT_HIGH] : (pos))

#define SERVO_POS_ABOVE_TR_DIR_NORM(ser,extr,fac,tr)\
            SERVO_REDUCE_TO_LIMIT_HIGH(ser, ((ser.adc < extr)?\
            (ser.pos+(fac*(ser.adc-tr)/tr)):ser.limit[LIMIT_HIGH]))

#define SERVO_POS_ABOVE_TR_DIR_REV(ser,extr,fac,tr)\
            SERVO_REDUCE_TO_LIMIT_LOW(ser, ((ser.adc < extr)?\
            (ser.pos-(fac*(ser.adc-tr)/tr)):ser.limit[LIMIT_LOW]))

#define SERVO_POS_BELOW_TR_DIR_NORM(ser,extr,fac,tr)\
            SERVO_REDUCE_TO_LIMIT_LOW(ser, ((ser.adc > extr)?\
            (ser.pos-(fac*(tr-ser.adc)/tr)):ser.limit[LIMIT_LOW]))

#define SERVO_POS_BELOW_TR_DIR_REV(ser,extr,fac,tr)\
            SERVO_REDUCE_TO_LIMIT_HIGH(ser, ((ser.adc > extr)?\
            (ser.pos+(fac*(tr-ser.adc)/tr)):ser.limit[LIMIT_HIGH]))


uint16_t servos_handler::get_position(servo_t &servo)
{
    static const uint16_t extreme_up = 2400, extreme_down = 50;
    static const uint16_t threshold = 1300, factor = 13;
    uint16_t pos = DATA_NOT_AVAIL;

    if(servo.adc > threshold) {
        if(servo.dir == DIR_NORMAL) {
	    pos = SERVO_POS_ABOVE_TR_DIR_NORM(servo, extreme_up, factor, threshold);
	}
	else {
	    pos = SERVO_POS_ABOVE_TR_DIR_REV(servo, extreme_up, factor, threshold);
	}
    }
    else {
	if(servo.dir == DIR_NORMAL) {
	    pos = SERVO_POS_BELOW_TR_DIR_NORM(servo, extreme_down, factor, threshold);
	}
	else {
	    pos = SERVO_POS_BELOW_TR_DIR_REV(servo, extreme_down, factor, threshold);
	}
    }

    return pos;
}

delta_res_t servos_handler::calc_servos_delta(servo_it s, pos_it p)
{
    int32_t delta[] = { 
        std::abs(s->pos - *p),
        std::abs((s+1)->pos - *(p+1))
    };         

    return (delta[0] > delta[1]) ? DELTA_MORE:DELTA_LESS;
}

bool servos_handler::is_tilt_conflict(servo_it servo, pos_it pos)
{
    if(servo->conflict_servo != SERVO_NONE) {
        auto conflict_servo = servos.begin() + servo->conflict_servo;
        int16_t tilt_main = (this->*(servo->tilt_hdlr))(servo, *pos);
        int16_t tilt_comp = conflict_servo->tilt;

        if((tilt_main^tilt_comp) > 0) {
            int16_t delta = std::abs(tilt_main + tilt_comp);

            if(delta > servo->conflict_tilt) {
                log(LOG_ERROR, __func__, "Invalid tilt ("+std::to_string(delta)\
                  +"°) for servos "+servo->name+"("+std::to_string(tilt_main)+"°), "
                  +conflict_servo->name+"("+std::to_string(tilt_comp)+"°)");


                uint16_t tmp_time = 0;
                for(uint16_t i = 3; i > 0; i--) {
                    //set_buzzer(true);
                    tmp_time = 0;
                    while(tmp_time++ < 25) {
                        rgb_set_color(RGB_RED, 10*tmp_time+5);
                        usleep(20);
                    }

                    //set_buzzer(false);
                    tmp_time = 25;
                    while(tmp_time--) {
                        rgb_set_color(RGB_RED, 10*tmp_time+5);
                        usleep(20);
                    }
                }
                rgb_set_color(RGB_RED, 255);
                return true;
            }
        }
    }
    rgb_set_color(RGB_GREEN, 0x3F);

    return false;
}

void servos_handler::extract_positions(pos_t &pos)
{
    auto s=servos.begin(), sEnd=servos.end();
    auto p=pos.begin(), pEnd=pos.end();

    for(; s != sEnd && p != pEnd; s++, p++) {
	if(s->adc != DATA_NOT_AVAIL) {
            *p = get_position(*s);
	}
    }
}

void servos_handler::detect_major_vector(pos_t &pos)
{
    const uint8_t step = SERVOS_PER_ADC; 
    auto s=servos.begin(), sEnd=servos.end();
    auto p=pos.begin(), pEnd=pos.end();

    for(; s != sEnd && p != pEnd; s+=step, p+=step) {
        if(*p != DATA_NOT_AVAIL && *(p+1) != DATA_NOT_AVAIL) {
            if(calc_servos_delta(s, p) == DELTA_MORE) {
                *(p+1) = DATA_NOT_AVAIL;
            }
            else {
                *p = DATA_NOT_AVAIL;
            }
        }
    }
}

void servos_handler::check_tilt_conflicts(pos_t &pos)
{
    auto s=servos.begin(), sEnd=servos.end();
    auto p=pos.begin(), pEnd=pos.end();

    for(; s != sEnd && p != pEnd; s++, p++) {
        if(*p != DATA_NOT_AVAIL && is_tilt_conflict(s, p) == true) {      
            *p = DATA_NOT_AVAIL;
        }
    }
}

void servos_handler::store_positions(pos_t &pos)
{
    auto s=servos.begin(), sEnd=servos.end();
    auto p=pos.begin(), pEnd=pos.end();

    for(; s != sEnd && p != pEnd; s++, p++) {
        if(*p != DATA_NOT_AVAIL) {      
            s->pos = *p;
            s->tilt = (this->*(s->tilt_hdlr))(s, s->pos);

            log(LOG_DEBUG, __func__, "Servo "+s->name+" has tilt: "
                +std::to_string(s->tilt)+"°");
        } 
    }
}

void servos_handler::trackAndUpdateThread(void)
{
    static const uint8_t delay_ms = 2;
    
    while(1) {
        std::unique_lock<std::mutex> lock(servo_mtx);
       
        if(read_servos_pos() == 0) { 
            pos_t pos(SERVOS_NB, DATA_NOT_AVAIL);

            extract_positions(pos);
            detect_major_vector(pos);
            check_tilt_conflicts(pos); 
            store_positions(pos);
            write_servos_pos();
        }
   
        lock.unlock();
        usleep(delay_ms*1000);
    }
}

void servos_handler::release_device(void)
{
    disengage_servos();
}

void servos_handler::reinit_device(void)
{
    union {
        uint8_t raw[2];
	uint16_t reg;
    } config;

    read_servos_config(config.raw, sizeof(config.raw));
    if(config.reg == CONFIG_REG_DEFAULT) {
	static const uint8_t reinit_delay_s = 3;

	for(uint16_t i = reinit_delay_s; i > 0; i--) {
            log(LOG_DEBUG, __func__, "Re-init in: "+std::to_string(i)+" sec");
	    sleep(1);
	}
	
        log(LOG_ERROR, __func__, "Reinitialization initiated");
	init_device();
    }
}

int16_t servos_handler::servo_pos_handler(servo_it servo, int16_t offset, bool reversed, uint16_t pos)
{
    int16_t ret = 0;
    uint16_t div = servo->limit[LIMIT_HIGH] - servo->limit[LIMIT_LOW];

    if(div != 0) {
        int32_t tmp = servo->end_tilt * (pos - servo->limit[LIMIT_LOW]);
        ret = (int16_t)(offset - tmp/div);
        ret = (reversed == false) ? ret:((-1)*ret); 
    }

    return ret;
}

int16_t servos_handler::servo_1_pos(servo_it servo, uint16_t pos)
{
    return servo_pos_handler(servo, 270, false, pos);
}

int16_t servos_handler::servo_2_pos(servo_it servo, uint16_t pos)
{
    return servo_pos_handler(servo, 85, false, pos);
}

int16_t servos_handler::servo_3_pos(servo_it servo, uint16_t pos)
{
    return servo_pos_handler(servo, 90, true, pos);
}

int16_t servos_handler::servo_4_pos(servo_it servo, uint16_t pos)
{
    return servo_pos_handler(servo, 0, true, pos);
}

