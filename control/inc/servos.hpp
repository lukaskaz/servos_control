#ifndef __SERVOS_H__
#define __SERVOS_H__

#include <vector>
#include <initializer_list>
#include <mutex>

#include "i2c.hpp"
#include "logs.hpp"
#include "helper.hpp"

#define DATA_NOT_AVAIL      0xFFFFU

class servos_handler;
struct servo_t;

typedef enum {
        LIMIT_LOW = 0,
        LIMIT_HIGH,
        LIMIT_LAST
} limit_t;

typedef enum {
        DIR_NORMAL = 0,
        DIR_REVERSED
} direction_t;

typedef enum {
    DELTA_UNDEF = 0,
    DELTA_LESS,
    DELTA_MORE
} delta_res_t;

typedef enum {
    SERVO_NONE = (-1),
    SERVO_FIRST,
    SERVO_1 = SERVO_FIRST,
    SERVO_2,
    SERVO_3,
    SERVO_4,
    SERVO_LAST
} servos_group;

using pos_t = std::vector<uint16_t>;
using pos_it = pos_t::const_iterator;
using servo_it = std::vector<servo_t>::const_iterator;
using tilt_hdlr_t = int16_t (servos_handler::*)(servo_it, uint16_t);

struct servo_t {
public:
    servo_t(const std::string &n, uint16_t in, 
            const std::initializer_list<uint16_t> &il, direction_t d,
            int16_t stil, int16_t etil, servos_group cser,
            int16_t ctil, tilt_hdlr_t hdlr) :
            name(n), init_pos(in), adc(DATA_NOT_AVAIL), pos(DATA_NOT_AVAIL),
            limit(il), dir(d), tilt(DATA_NOT_AVAIL), start_tilt(stil),
            end_tilt(etil), conflict_servo(cser), conflict_tilt(ctil),
            tilt_hdlr(hdlr) {}

    std::string name;
    const uint16_t init_pos;

    uint16_t adc;
    uint16_t pos;
    pos_t limit;
    direction_t dir;

    int16_t tilt;
    int16_t start_tilt;
    int16_t end_tilt;

    servos_group conflict_servo;
    int16_t conflict_tilt;     

    tilt_hdlr_t tilt_hdlr;
};

class servos_handler : 
    virtual private Log,
    private I2C,
    private gpio_handler
{
public:
    servos_handler(): Log(false, true, true, true), I2C("/dev/i2c-1", 0x40)
                      { init(); }
    ~servos_handler() { release_device(); }

    void run(void);  

private:
    static std::mutex servo_mtx;
    std::vector<servo_t> servos;

    void init(void);
    void init_data(void);
    void init_device(void);
    void release_device(void);

    void reinit_device(void);

    void energise_servos(void);
    void disengage_servos(void);
    void init_servos_driver(void);
    void set_servos_init_position(void);

    int8_t read_servos_adcs(void);
    int8_t read_servos_pos(void);
    void write_servos_pos(void);
    void write_servos_all_pos(void);
    void read_servos_config(uint8_t*, size_t);
    void set_servos_config(uint8_t, uint8_t);

    uint16_t get_position(servo_t&);
    delta_res_t calc_servos_delta(servo_it, pos_it);
    bool is_tilt_conflict(servo_it, pos_it);
    void extract_positions(pos_t &);
    void detect_major_vector(pos_t &);

    void check_tilt_conflicts(pos_t &);
    void store_positions(pos_t &);
    void trackAndUpdateThread(void);

    int16_t servo_pos_handler(servo_it, int16_t, bool, uint16_t);

    int16_t servo_1_pos(servo_it, uint16_t);
    int16_t servo_2_pos(servo_it, uint16_t);
    int16_t servo_3_pos(servo_it, uint16_t);
    int16_t servo_4_pos(servo_it, uint16_t);

    bool is_emerg_active(void);
};


#endif
