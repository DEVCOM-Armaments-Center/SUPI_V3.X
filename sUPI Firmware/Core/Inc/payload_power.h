/*****************************************************************************
 * \brief Payload power control and monitoring for the small Universal
 * Platform Interface
 * \date May 2023
 *
 * Part of the sUPI v3.3 reference design. See LICENSE.md at the repository
 * root.
 */
#ifndef PAYLOAD_POWER_H
#define PAYLOAD_POWER_H
 
/* Includes */
#include <stdint.h>
#include <stdbool.h>

/* Defines */
/* Derivation for maximum value:
 *   * Zero current results in mid-scale reading (1.65V)
 *   * Sensitivity is 90mV/A
 *   * Reference voltage for ADC: 3.3V
 *   * Max ADC counts: 4095
 *
 *     counts = ceil((((amps * sensitivity) + baseline) / Vref ) * (max ADC counts))
 *
 * Precomputed counts for different currents:
 *   10A: 3165
 *   9A: 3053
 *   8A: 2941
 *   7A: 2830
 *   6A: 2718
 *   5A: 2606
 *   4A: 2495
 *   3A: 2383
 *   2A: 2271
 *   1A: 2160
 */

/*
 * The watchdog will cause the current to be actively monitored
 * to ensure it does not exceed a hard limit.
 *
 * This is set conservatively high because the watchdog can catch
 * transients on the order of sub-microseconds.
 * 
 * NOTE: this actually needs to be set in the STM32CubeMX utility
 * since it goes into auto-generated code.
 */
#define PAYLOAD_POWER_ADC_I_SNS_WATCHDOG_HI_COUNTS  3165

/* At this point, we are not likely to see negative currents so this
 * side of the watchdog will be ignored.
 */
#define PAYLOAD_POWER_ADC_I_SNS_WATCHDOG_LO_COUNTS  0

/* "Slow-blow fuse" refers to a software limit on output current that
 * tolerates a short period of excess current but eventually disables
 * the output if that persists.  A "fast fuse" is implemented by
 * an ADC watchdog that immediately cuts power if a (higher) current
 * threshold is ever breached.
 */
/* Comment this line to disable slow-blow fuse behavior */
#define PAYLOAD_POWER_SLOW_BLOW_FUSE

/* Derivation for slow blow is same as above for 4A.
 * This will be used both for short bursts and average current.
 */
#define PAYLOAD_POWER_ADC_I_SNS_SLOW_BLOW_HI_COUNTS     2495
#define PAYLOAD_POWER_ADC_I_SNS_POLL_DELAY_MS           10
#define PAYLOAD_POWER_SLOW_BLOW_VIOLATION_LIMIT         10      /* Around 100ms */

#define PAYLOAD_POWER_SLOW_BLOW_AVERAGING_SAMPLE_COUNT  100 /* Average over around 1s */

/* Function prototypes */

/**
 * \brief Initialize the payload power interface.  This must be
 * done prior to calling any of the other payload_power functions.
 */
void payload_power_initialize();

/**
 * \brief Function to report that the detection status of the payload
 * has changed.  This may be called due to actual connection and
 * disconnection actions by the user, as well as at startup when the
 * presence of the payload is first probed.
 *
 * \param[in] detected true if the payload has been detected,
 * false otherwise.
 */
void payload_power_notify_payload_changed(bool detected);


/**
 * \brief Prompt the payload power interface to perform any
 * periodic tasks that it needs to do
 */
void payload_power_update();

#endif /* PAYLOAD_POWER_H */
