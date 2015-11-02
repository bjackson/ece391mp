#include "rtc.h"

#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT 0x71
#define MIN_RATE 2
#define MAX_RATE 15

/*
 * Initializes the RTC
 * INPUTS: none
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_init() {
    
    // disable interupts to keep RTC from entering an undefined state
    disable_ints();
    outb(0x0B, RTC_INDEX_PORT)
    uint8_t previous = inb(RTC_DATA_PORT):
    outb(0x0B, RTC_INDEX_PORT);
    outb(previous | 0x40, RTC_DATA_PORT);
    
    // set default frequency
    rtc_set_frequency(2);
    
    // enable interrupts again
    enable_inits();
    
    return 0;
}

/*
 * Sets the RTC frequency
 * INPUTS: rate - the rate at which to create the new clock
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_set_frequency(int32_t rate) {
    // test if valid frequency. uses clever trick for power of 2 from: http://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2
    if (rate < MIN_RATE || rate > MAX_RATE || (rate & (rate - 1))) {
        return -1;
    }
 
    // format rate. from OSDev
    int32_t frequency = 32768 >> (rate - 1);
    
    
    // send new clock to registers
    disable_ints();
    outb(0x0A, RTC_INDEX_PORT)
    uint8_t previous = inb(RTC_DATA_PORT):
    outb(0x0A, RTC_INDEX_PORT);
    outb((previous & 0xF0) | frequency, RTC_DATA_PORT);
    enable_inits();
    
    return 0;
    
}

/*
 * Open - does nothing now
 * INPUTS: filename - name of the file
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_open(const uint8_t* filename) {
    return 0;
}

/*
 * Close - resets the frequency
 * INPUTS: fd - garbage
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_close(int32_t fd) {
    // reset frequency to 2
    rtc_set_frequency(2);
    
    return 0;
}

/*
 * Read
 * INPUTS: fd, buf, nbytes - all garbage
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    uint32_t current_ticks = tick_counter;
    
    // spin until interrupt
    while (current_ticks == tick_counter) {}
    return 0;
}

/*
 * Write the new frequency to the rtc
 * INPUTS: rfd - garbage, buf - data, nbytes - size of data
 * OUTPUTS: none
 * RETURNS: 0 on success, -1 on failure
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    
    int32_t frequency;
    
    // check for valid values
    if (buf == NULL || nbytes <= 0) {
        return -1;
    }
    
    // case for nbytes - cast the void appropriately
    switch (nbytes) {
        case 1:
            frequency = *buf;
            break;
        case 2:
            frequency = *(int16_t *)buf;
            break;
        case 4:
            frequency = *(int32_t *)buf;
            
            
        default:
            return -1;
            break;
    }
    
    return rtc_set_frequency(frequency);
    
}