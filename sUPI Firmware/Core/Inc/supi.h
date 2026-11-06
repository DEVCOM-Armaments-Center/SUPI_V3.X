/*****************************************************************************
 * \brief Definitions related to the small Universal Platform Interface
 * \date May 2023
 *
 * Part of the sUPI v3.3 reference design. See LICENSE.md at the repository
 * root.
 */
#ifndef SUPI_H
#define SUPI_H

/* Includes */
#include <stdint.h>

/* Defines */

/* This defines how long to wait between attempts to detect a payload. */
#define SUPI_PAYLOAD_DETECT_REPEAT_PERIOD_MSEC      50

/* Debounce is the number of consecutive detection results needed to change
 * whether or not the payload is considered to be attached.  A higher value
 * provides more tolerance for transients, but means the payload power may be
 * left on for longer when it has really been disconnected.
 */
#define SUPI_PAYLOAD_DETECT_DEBOUNCE_COUNT          5

/* Which One Wire bus to use for detecting the payload */
#define SUPI_PAYLOAD_DETECT_ONE_WIRE_BUS_INDEX      0

/* Function prototypes */

/**
 * \brief Initialize the small Universal Platform Interface application
 */
void supi_initialize();

/**
 * \brief Handler that should be called by periodic SysTick interrupts as set up
 * by the HAL
 */
void supi_systick_handler();

#endif /* SUPI_H */
