
#include "rtc.h"
#include "../lib.h"
#include "../interrupts/interrupts.h"
#include "../types.h"

/**
 * Initializes the RTC
 * INPUTS: none
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_init() {
    // Set default frequency
    rtc_set_frequency(2);

    disable_inits(); // Disable interupts to keep RTC from entering an undefined state

    outb(RTC_REGB, RTC_INDEX_PORT);
    uint8_t previous = inb(RTC_DATA_PORT);
    outb(RTC_REGB, RTC_INDEX_PORT);
    outb(previous | 0x40, RTC_DATA_PORT);

    enable_irq(RTC_IRQ); // Enable RTC interrupt
    enable_inits(); // Enable interrupts again

    tick_counter = 0;

    return 0;
}

/**
 * Sets the RTC frequency
 * INPUTS: freq - the frequency at which to create the new clock
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_set_frequency(int32_t freq) {
    // Test if valid frequency. Uses clever trick for power of 2 from: http://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2
    if(freq < MIN_FREQ || freq > MAX_FREQ || (freq & (freq - 1))) {
        return -1;
    }

    // Based on Table 3 in RTC datasheet
    uint8_t rate = 16 - log2_of_pwr2(freq);

    disable_inits();

    // Send new clock to registers
    outb(RTC_REGA, RTC_INDEX_PORT);
    uint8_t previous = inb(RTC_DATA_PORT);
    outb(RTC_REGA, RTC_INDEX_PORT);
    outb((previous & 0xF0) | rate, RTC_DATA_PORT);

    enable_inits();

    return 0;
}

/**
 * Open - does nothing now
 * INPUTS: filename - name of the file
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_open(const uint8_t* filename) {
    return 0;
}

/**
 * Close - resets the frequency
 * INPUTS: fd - garbage
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_close(int32_t fd) {
    // Reset frequency to 2
    rtc_set_frequency(2);
    return 0;
}

/**
 * Read
 * INPUTS: fd, buf, nbytes - all garbage
 * OUTPUTS: none
 * RETURNS: 0 on success
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    uint32_t current_ticks = tick_counter;

    // spin until interrupt
    while(current_ticks == tick_counter);
    return 0;
}

/**
 * Write the new frequency to the rtc
 * INPUTS: rfd - garbage, buf - data, nbytes - size of data
 * OUTPUTS: none
 * RETURNS: 0 on success, -1 on failure
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    int32_t frequency;

    // Check for valid values
    if(buf == NULL || nbytes <= 0) {
        return -1;
    }

    // Case for nbytes - cast the void appropriately
    switch (nbytes) {
        case 1:
            frequency = *(int8_t *)buf;
            break;
        case 2:
            frequency = *(int16_t *)buf;
            break;
        case 4:
            frequency = *(int32_t *)buf;
            break;
        default:
            return -1;
    }

    return rtc_set_frequency(frequency);
}

