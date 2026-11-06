/*****************************************************************************
 * \brief Payload power control and monitoring for the small Universal
 * Platform Interface
 * \date May 2023
 *
 * Part of the sUPI v3.3 reference design. See LICENSE.md at the repository
 * root.
 */
 
/* Includes */
#include "payload_power.h"
#include "main.h"
#include <stdint.h>

/* Local types */
typedef enum {
    PAYLOAD_POWER_DISABLED = 0,
    PAYLOAD_POWER_ENABLED = 1,
} payload_power_state_t;

/* File-scoped function prototypes */
static void __enable_payload_power();
static void __disable_payload_power();
static void __setup_current_sense();

/* TODO: would prefer to have this be file-scoped, but the setup
 * and calibration are handled elsewhere.  May want to set up an
 * accessor function for this later instead of direct linkage.
 */
extern ADC_HandleTypeDef hadc1;

/* File-scoped variables */
static payload_power_state_t __payload_power_state = PAYLOAD_POWER_DISABLED;
static uint16_t __payload_current_samples[PAYLOAD_POWER_SLOW_BLOW_AVERAGING_SAMPLE_COUNT] = { 0 };
static uint16_t __payload_current_sample_next_index = 0;

void payload_power_initialize() {
    /* For now, this will assume that MX_GPIO_Init() has been called, and
     * all of the defaults are sane (including the default load switch
     * enable pin is driving logic 0).  If for some reason this is ever
     * called into question, the various pins can be explicitly
     * reconfigured here.
     */
    
    __setup_current_sense();
}

void payload_power_notify_payload_changed(bool detected) {
    /* This is not particularly sophisticated yet.  If a payload is detected,
     * it will be given power.  Additional logic could be added here if,
     * for instance, a command was received on a digital bus that overrode
     * the power control to ignore the detection status.
     */
    if(true == detected) {
        __enable_payload_power();
    } else {
        __disable_payload_power();
    }
}

void payload_power_update() {
    /* This is called periodically from a low-priority interrupt context */
#ifdef PAYLOAD_POWER_SLOW_BLOW_FUSE
    uint32_t counts;
    uint32_t index;
    static uint32_t consecutive_violations = 0;
    
    counts = HAL_ADC_GetValue(&hadc1);
    if(counts > PAYLOAD_POWER_ADC_I_SNS_SLOW_BLOW_HI_COUNTS) {
        consecutive_violations++;
    } else {
        consecutive_violations = 0;
    }
    
    /* Add the latest counts to the log of samples */
    __payload_current_samples[__payload_current_sample_next_index] = counts;
    /* Increment and wrap as needed.  Note that at startup, most samples will be zero so this
     * will take some time to settle in.  Combined with the violation counts above, this should
     * provide decent guard rails.
     */
    __payload_current_sample_next_index =
        (__payload_current_sample_next_index + 1) % PAYLOAD_POWER_SLOW_BLOW_AVERAGING_SAMPLE_COUNT;
    
    /* Now check for average current over the sampling period */
    for(counts = 0, index = 0; index < PAYLOAD_POWER_SLOW_BLOW_AVERAGING_SAMPLE_COUNT; index++) {
        counts += __payload_current_samples[index];
    }
    counts /= PAYLOAD_POWER_SLOW_BLOW_AVERAGING_SAMPLE_COUNT;   /* Find the mean */

    if(consecutive_violations >= PAYLOAD_POWER_SLOW_BLOW_VIOLATION_LIMIT
            || counts >= PAYLOAD_POWER_ADC_I_SNS_SLOW_BLOW_HI_COUNTS) {
        /* Either there were too many violations in a short period of time,
         * or the average current over a longer interval has risen too high
         */
        __disable_payload_power();
        /* TODO: if automatic restart was desired, some sort of state
         * machine would need to be advanced here
         */
        return; /* Just for breakpoints */
    }

#endif /* PAYLOAD_POWER_SLOW_BLOW_FUSE */
}

void __enable_payload_power() {
    HAL_GPIO_WritePin(LS_ENABLE_GPIO_Port, LS_ENABLE_Pin, GPIO_PIN_SET);
}

void __disable_payload_power() {
    HAL_GPIO_WritePin(LS_ENABLE_GPIO_Port, LS_ENABLE_Pin, GPIO_PIN_RESET);
}

void __setup_current_sense() {
    /*
     * There are two current sensors, one in the Hall Effect sensor, and one in
     * the load switch.  The load switch sensor has very loose specifications,
     * which is why the Hall Effect sensor was added.
     *
     * The I_SNS line is monitored using the ADC.  This is used to control two
     * kinds of safety behaviors: "fast fuse" and "slow fuse".  The "fast fuse"
     * is implemented as an ADC watchdog that will cut power if there is a single
     * violation of its threshold since this is a threat to the hardware
     * (e.g. dead short on the connector).  The "slow fuse" tolerates transients
     * that exceed its threshold, but which if allowed to continue indefinitely
     * could cause damage.
     *
     * There are two aspects to the "slow fuse": current that continuously (within
     * the resolution of the sampling rate) exceeds the threshold for some period
     * of time (that is, allowing for inrush or short bursts but cutting off after
     * a delay), and current that on average exceeds the same threshold over a longer
     * interval (such as one second).  The latter case is meant to catch situations
     * in which the current might flow at a high duty cycle but is interrupted just
     * often enough not to be detected as an overly-long burst.  "Slow fuse" should
     * prevent gradual but continuous heating of the interface to unsafe levels.
     *
     * The ADC must be running for those to work.  The watchdog will react immediately
     * for the "fast fuse", and a SysTick-based periodic poll will be used for the
     * "slow fuse".
     *
     *
     * The IMON line from the load switch can also be monitored, but it will be of
     * secondary importance.
     */
     
     /* Enable interrupts for watchdog 2 window violations */
     __HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_AWD2);
     
     /* Start sampling current */
     HAL_ADC_Start(&hadc1);
}

void HAL_ADCEx_LevelOutOfWindow2Callback(ADC_HandleTypeDef *adc) {
    /* ADC1 is used to monitor payload current */
    /* This watchdog is set up to catch events
     * that greatly exceed the current rating of the pins
     * between the UPI boards, e.g. a dead short.  If this
     * occurs, the power needs to be disconnected as quickly
     * as possible.
     */
    
    /* Turn off power to protect the hardware */
    __disable_payload_power();
    
    /* TODO: should this wait for the user to remove and
     * reconnect the payload, thus causing the detection state
     * machine to start over and possibly turn power back on,
     * try to restart on its own (perhaps after a delay),
     * or just latch off until fully reset?  If nothing special
     * is done here, reconnecting the payload should cause it to
     * try again.
     */
    
    /* This assumes that software will periodically check the ADC
     * value from a different (e.g. SysTick) interrupt rather than
     * after every single conversion for purposes such as a
     * "slow fuse" behavior and for reporting typical values.
     */
}