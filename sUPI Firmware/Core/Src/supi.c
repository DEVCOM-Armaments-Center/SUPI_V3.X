/*****************************************************************************
 * \brief Application code for the small Universal Platform Interface
 * \date May 2023
 *
 * Part of the sUPI v3.3 reference design. See LICENSE.md at the repository
 * root.
 */
 
/* Includes */
#include "supi.h"
#include "stm32c0x1_one_wire.h"
#include "payload_power.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/* Local types */
typedef enum {
    SUPI_STATE_INITIALIZING = 0,
    SUPI_STATE_RUNNING = 1
} supi_state_t;

typedef struct {
    one_wire_presence_result_t state;
    one_wire_presence_result_t trend;
    uint32_t ticks;
    uint32_t consecutive_results;
} supi_payload_detection_tracker_t;

/* File-scoped variables */
static supi_state_t __supi_state = SUPI_STATE_INITIALIZING;
static supi_payload_detection_tracker_t __payload_detect = {
    .state = ONE_WIRE_PRESENCE_UNDEFINED,
    .trend = ONE_WIRE_PRESENCE_UNDEFINED,
    .ticks = 0,
    .consecutive_results = 0,
};
static uint32_t __payload_power_ticks = 0;

/* File-scoped prototypes */
static void __one_wire_presence_callback(one_wire_presence_result_t result);
static void __supi_notify_payload_changed(bool detected);

void supi_initialize() {
    /* Initialize the payload power control and monitoring interface */
    payload_power_initialize();
    
    /* Initialize the One Wire bus for detecting payloads */
    stm32c0x1_one_wire_initialize();
    stm32c0x1_one_wire_initialize_bus(SUPI_PAYLOAD_DETECT_ONE_WIRE_BUS_INDEX);
    
    __supi_state = SUPI_STATE_RUNNING;
}

void supi_systick_handler() {
    /* uwTickFreq is defined by the HAL to be in units of milliseconds */
    extern uint32_t uwTickFreq;
    
    if(SUPI_STATE_RUNNING != __supi_state) {
        /* Not ready to handle any events yet */
        return;
    }
    
    __payload_detect.ticks += uwTickFreq;
    __payload_power_ticks += uwTickFreq;
    /* TODO: update other tick counters if needed */
    
    if(__payload_detect.ticks >= SUPI_PAYLOAD_DETECT_REPEAT_PERIOD_MSEC) {
        __payload_detect.ticks = 0;
        stm32c0x1_one_wire_reset_and_detect_presence(
            SUPI_PAYLOAD_DETECT_ONE_WIRE_BUS_INDEX, __one_wire_presence_callback);
    }
    
    if(__payload_power_ticks >= PAYLOAD_POWER_ADC_I_SNS_POLL_DELAY_MS) {
        /* Sense output current, etc. */
        payload_power_update();
    }
}

void __one_wire_presence_callback(one_wire_presence_result_t result) {
    /* NOTE: this gets called from an interrupt context and should return as soon
     * as possible.
     */
    if(ONE_WIRE_PRESENCE_DETECTED != result && ONE_WIRE_PRESENCE_NOT_DETECTED != result) {
        /* TODO: this would almost certainly indicate a bug.  Put a trap here if it
         * appears to actually be happening.
         */
        return;
    }
    
    if(result != __payload_detect.trend) {
        /* The trend changed */
        __payload_detect.consecutive_results = 0;
    }
    
    __payload_detect.trend = result;
    
    if(__payload_detect.consecutive_results < 1000U) {
        /* Increment the counter, but bound it to avoid overflows.  Anything above
         * the debounce threshold does not provide any new information.
         */
        __payload_detect.consecutive_results++;
    }
    
    if(__payload_detect.state != __payload_detect.trend
            && __payload_detect.consecutive_results >= SUPI_PAYLOAD_DETECT_DEBOUNCE_COUNT) {
        /* The payload state seems to have changed persistently */
        __payload_detect.state = __payload_detect.trend;
        __supi_notify_payload_changed(ONE_WIRE_PRESENCE_DETECTED == __payload_detect.state ? true : false);
    }
}

static void __supi_notify_payload_changed(bool detected) {
    payload_power_notify_payload_changed(detected);
}

