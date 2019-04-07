#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "helper.hpp"

int8_t gpio_handler::init(void)
{
    gpio = (gpio_t*)mapmem(GPIO_OFFSET + platform, sizeof(gpio_t), DEV_MEM);
    if(gpio == NULL) {
        return (-1);
    }
    return 0;
}

int8_t gpio_handler::destroy(void)
{
    if(gpio != NULL) {
        unmapmem((void *)gpio, sizeof(gpio_t));
	return 0;
    }
    return (-1);
}


void gpio_handler::set_relay(uint8_t state)
{
    gpio_function_set(gpio, 27, 6);

    if(state) {
        log(LOG_DEBUG, __func__, "Activating relay");
	gpio_level_set(gpio, 27, 0);
    }
    else {
        log(LOG_DEBUG, __func__, "Deactivating relay");
        gpio_level_set(gpio, 27, 1);
    }
}

void gpio_handler::switch_relay(void)
{
    uint8_t value = 0xFF;
    gpio_function_set(gpio, 27, 6);
    gpio_level_get(gpio, 27, &value);

    if(value == 1) {
        log(LOG_DEBUG, __func__, "Activiatng relay");
	gpio_level_set(gpio, 27, 0);
    }
    else {
        log(LOG_DEBUG, __func__, "Deactivating relay");
        gpio_level_set(gpio, 27, 1);
    }
}

uint8_t gpio_handler::get_emerg_state(void)
{
        uint8_t value = 0xFF;

	gpio_function_set(gpio, 5, 7);
        gpio_level_get(gpio, 5, &value);
	return value;
}

uint8_t gpio_handler::get_relay_state(void)
{
        uint8_t value = 0xFF;

        gpio_level_get(gpio, 27, &value);
	return value;
}

void gpio_handler::set_buzzer(uint8_t state)
{

	if(state) {
                fprintf(stderr, "[%s] Activiating buzzer\n", __func__);
                gpio_function_set(gpio, 6, 6);
                gpio_level_set(gpio, 6, 1);
        }
        else {
                fprintf(stderr, "[%s] Deactiviating buzzer\n", __func__);
                gpio_function_set(gpio, 6, 6);
                gpio_level_set(gpio, 6, 0);
                //gpio_function_set(gpio, 6, 7);
        }
}

void *gpio_handler::mapmem(uint32_t base, uint32_t size, const char *mem_dev)
{
    uint32_t pagemask = ~0UL ^ (getpagesize() - 1);
    uint32_t offsetmask = getpagesize() - 1;
    int mem_fd;
    void *mem;

    mem_fd = open(mem_dev, O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("Can't open /dev/mem");
        return NULL;
    }

    mem = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, base & pagemask);
    if (mem == MAP_FAILED) {
        perror("mmap error\n");
	return NULL;
    }

    close(mem_fd);

    return (char *)mem + (base & offsetmask);
}

void *gpio_handler::unmapmem(void *addr, uint32_t size)
{
    uint32_t pagemask = ~0UL ^ (getpagesize() - 1);
    uintptr_t baseaddr = (uintptr_t)addr & pagemask;
    int s;

    s = munmap((void *)baseaddr, size);
    if (s != 0) {
        perror("munmap error\n");
    }

    return NULL;
}

