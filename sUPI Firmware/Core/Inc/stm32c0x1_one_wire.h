/*****************************************************************************
 * \brief Constants for (formerly Dallas Semiconductor) One-Wire serial
 * bus using timers on STM32C0x1 (and possibly other) microcontroller timers
 * \date May 2023
 *
 * Part of the sUPI v3.3 reference design. See LICENSE.md at the repository
 * root.
 */
#ifndef STM32C0X1_ONE_WIRE_H
#define STM32C0X1_ONE_WIRE_H

/* Includes */
#include "one_wire.h"
#include <stdint.h>

/* Types */
typedef void (*stm32c0x1_one_wire_presence_callback_fn)(one_wire_presence_result_t result);

/* Public function prototypes */

/**
 * \brief Initialize the one-wire driver.  This must be called before any other
 * stm32c0x1_ functions.
 * \return 0 on success, others on error
 */
int stm32c0x1_one_wire_initialize();

/**
 * \brief Get the number of one wire buses available.  This defines the limit
 * of what the maximum bus index can be in subsequent functions.
 * \return Number of one-wire buses configured on the system
 */
uint8_t stm32c0x1_one_wire_get_bus_count();

/**
 * \brief Initialize the given one-wire bus.  This must be called before any other
 * stm32c0x1_ functions that take a bus as a parameter.
 * \param bus One-wire bus to initialize
 * \return 0 on success, others on error
 */
int stm32c0x1_one_wire_initialize_bus(uint8_t bus_index);

/**
 * \brief Abort any ongoing process and return the bus to the idle state
 * \param bus One-wire bus to abort
 */
void stm32c0x1_one_wire_abort(uint8_t bus_index);

/**
 * \brief Send a reset strobe to the one-wire bus and detect whether any
 * devices are present on the bus.  This will return immediately, but the
 * reset and detection process will take hundreds of microseconds to complete.
 * A callback function can be provided to alert when this is finished.
 * \param One-wire bus to reset and check for device presence
 * \param callback Function that will be invoked once the presence detection.
 * Note that this function will be invoked from an interrupt context and should
 * return as soon as possible.
 * \return 0 on success, others on error
 */
int stm32c0x1_one_wire_reset_and_detect_presence(uint8_t bus_index,
        stm32c0x1_one_wire_presence_callback_fn callback);

/* TODO: create functions for:
    * write 0
    * write 1
    * read
    NOTE: the immediate need for this driver was simply to detect whether
    a device was connected, so the full protocol was not needed.  This can
    be added once more sophisicated communication is required.
 */

#endif /* STM32C0X1_ONE_WIRE_H */
 