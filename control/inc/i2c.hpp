#ifndef __I2C_H__
#define __I2C_H__

#include <mutex>

#include "types.hpp"
#include "logs.hpp"

class I2C:
    virtual public Log
{
public:
    enum Req_t { REQ_UNDEF = 0, REQ_READ, REQ_WRITE };

    I2C(const std::string& bus, uint8_t addr): Log(false, true, true, true),
                                    i2c_bus(bus), i2c_addr(addr) { };
    int8_t i2c_transfer(Req_t req, uint8_t* data, const uint8_t size);

private:
    I2C(): I2C("", 0) { };

    const std::string i2c_bus;
    const uint8_t i2c_addr;
    static std::mutex i2c_mtx;
   
    int8_t i2c_read(int32_t fd, uint8_t* data, const uint8_t size);
    int8_t i2c_write(int32_t fd, uint8_t* data, const uint8_t size);
};

#endif
