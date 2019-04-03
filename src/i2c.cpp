#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <poll.h>

#include "i2c.hpp"

using i2c_transfer_fn_t = int8_t (I2C::*)(int32_t, uint8_t*, const uint8_t);
std::mutex I2C::i2c_mtx; 

int8_t I2C::i2c_read(int32_t fd, uint8_t* data, const uint8_t size)
{
    int8_t ret = (-1);

    if(data != NULL && size > 1) {
        if(i2c_write(fd, data, 1) == 0) {
            struct pollfd fds = { fd, POLLIN };
	    
            ret = poll(&fds, 1, 100);
	    if((ret > 0) && (fds.revents & POLLIN)) { 
            	if(read(fd, data, size) == size) {
                    ret = 0;
            	}
            	else {
                    log(LOG_ERROR, __func__, "Failed to read all data");
            	}
	    }
        }
        else {
            log(LOG_ERROR, __func__, "Failed to write all data");
        }
    }

    return ret;
}

int8_t I2C::i2c_write(int32_t fd, uint8_t* data, const uint8_t size)
{
    int8_t ret = (-1);

    if(data != NULL && size > 0) {
    	struct pollfd fds = { fd, POLLOUT };
	
	ret = poll(&fds, 1, 100);                                       
        if((ret > 0) && (fds.revents & POLLOUT)) {                      
       	    if(write(fd, data, size) == size) {
            	ret = 0;
            }
            else {
            	log(LOG_ERROR, __func__, "Failed to write all data");
            }
	}
    }

    return ret;
}

int8_t I2C::i2c_transfer(Req_t req, uint8_t* data, const uint8_t size)
{
    int8_t ret = (-1);
    int32_t fd = (-1);
    static const i2c_transfer_fn_t i2c_fn[] = 
    {
       [REQ_UNDEF] = NULL,
       [REQ_READ]  = &I2C::i2c_read,
       [REQ_WRITE] = &I2C::i2c_write
    };
    static const uint8_t i2c_fn_last = sizeof(i2c_fn)/sizeof(*i2c_fn);

    if (req < i2c_fn_last && i2c_fn[req] != NULL) {
        if(i2c_bus.empty() == false && i2c_addr != 0) {
            std::unique_lock<std::mutex> lock(i2c_mtx);
           
            fd = open(i2c_bus.c_str(), O_RDWR);
            if(fd >= 0) {

                if(ioctl(fd, I2C_SLAVE, i2c_addr) >= 0) {
                    ret = (this->*i2c_fn[req])(fd, data, size);
                }
                else {
                    log(LOG_ERROR, __func__, "Failed to prepare i2c to " \
                        + std::string("communicate to a slave device with addr: ") \
                        + numToString(NUM_HEX, i2c_addr));
                }

                close(fd);
                lock.unlock();
            }
            else {
                log(LOG_ERROR, __func__, "Failed to open bus: " + i2c_bus);
            }
        }
    }
    
    return ret;
}

