/*****************************************************************************
 * \brief Constants for (formerly Dallas Semiconductor) One-Wire serial bus
 * \date May 2023
 *
 * Part of the sUPI v3.3 reference design. See LICENSE.md at the repository
 * root.
 */
#ifndef ONE_WIRE_H
#define ONE_WIRE_H

/* Types */
typedef enum {
    ONE_WIRE_PRESENCE_UNDEFINED = 0,        /* No information about devices */
    ONE_WIRE_PRESENCE_NOT_DETECTED = 1,     /* No devices responded to probe */
    ONE_WIRE_PRESENCE_DETECTED = 2,         /* At least one device found */
} one_wire_presence_result_t;

#endif /* ONE_WIRE_H */
 