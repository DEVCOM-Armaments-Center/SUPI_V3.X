/*****************************************************************************
 * \brief Implementation of (formerly Dallas Semiconductor) One-Wire serial
 * bus using timers on STM32C0x1 (and possibly other) microcontroller timers
 * \date May 2023
 *
 * Part of the sUPI v3.3 reference design. See LICENSE.md at the repository
 * root.
 */
 
 /* The One-Wire protocol is highly timing-dependent.  The Standard speed
  * requires accurately driving and responding to edges on the tens
  * of microseconds time scale, and Overdrive speed requires activity
  * in single-digit microseconds.  This can be surprisingly challenging
  * to accomplish in an interrupt-driven fashion on a microcontroller
  * (as opposed to just polling for everything).
  *
  * The approach taken here is to use hardware timers to offload
  * the processor.  Master reset and write delays are implemented as
  * a simple interval timer.
  *
  * Detection of the falling edge that indicates the presence of a slave
  * device is performed using input capture so that the levels on both
  * sides of the transition do not have to be detected individually.
  * Changing modes of the pin from GPIO output to timer input requires
  * about 14usec (without compiler optimization) which is acceptable for
  * Standard speed (requirement is 15usec, typical is closer to 20usec)
  * but far too slow for Overdrive speed (1-2usec).  Note that the STM32Cube
  * HAL is far too bloated to perform at these speeds and direct register
  * writes are generally required.
  *
  * Reading from a slave device requires that the master pull the data line
  * low and then release it, and the slave can either continue to hold it
  * (data 0) or not (data 1).  Detecting the rising edge in a data 1 case
  * using input capture (as with the reset/detect handshake) would require
  * an instantaneous transition from GPIO output to timer input which is
  * not possible, so on a read operation, the data line must be sampled after
  * a delay.
  *
  * For the sake of simplicity, it may be desirable to replace the presence
  * detect falling edge capture with the same kind of delayed sampling used
  * to read data from the device.  There are a few tradeoffs to such a change.
  * It would reduce the setup cost for input capture and may free up a timer
  * peripheral.  However, it also introduces timing non-determinism, where
  * the processor could be interrupted at a critical time and miss an event.
  * Given that timing is everything to the One-Wire bus, this would need to
  * be approached carefully.
  *
  * TODO:
  * - Read and write capabilities have not been implemented.  These will require
  *   at least the following:
  *     - State machine will need to be expanded to include read and write states
  *     - Tracking of word transfers will need to be added, including bits read
  *       so far and the current bit count
  *     - Explicit sampling of data line, possibly to include glitch filtering,
  *       during master sample period for read cycles
  *   Read/write functionality would be helpful if payload identification was
  *   necessary.  For instance, payloads with only 8 pins could be identified
  *   and the current limit could be reduced accordingly.
  *
  * ADDENDUM:
  * The implementation described above is more or less required when a single
  * timer-attached pin is used to operate the 1-Wire bus.  However, an alternate
  * approach is to use the Tx and Rx lines of a UART peripheral.  This requires
  * that the hardware be designed correctly for 1-Wire emulation: the Tx line
  * must be pulled up and set as open drain, and the Rx line must be connected to
  * the Tx line and the 1-Wire bus (with appropriate resistors).  Using carefully
  * chosen baud rates, the UART transmits a specific byte to emulate a reset and
  * detect pulse, and the UART Rx receives the wire-AND combination of the Tx
  * byte and the 1-Wire slave output, if any.  Differences between Tx and Rx are
  * decoded to determine the 1-Wire bit value.  The same approach (probably using
  * a different baud rate) can be used for 1-Wire transmit and receive.  For more details, see
  * https://www.analog.com/en/resources/technical-articles/using-a-uart-to-implement-a-1wire-bus-master.html
  *
  * Using the UART peripheral to manage timing offloads the real-time sampling
  * requirement from the processor core and is the preferred way to go if the
  * board design supports it.  To handle transmit and receive operations, some sort
  * of state machine is required to respond to each newly received byte, compare it to
  * the transmitted value (necessary for read, nice-to-have for detecting bus errors
  * on write), and set up the next 1-Wire bit transfer.  The preferred way to manage
  * all of this context is within an RTOS that can block a task until an entire transfer is
  * complete, which allows higher-level drivers (such as an EEPROM protocol driver)
  * to operate without being closely tied to the bus.  At present, the STM32C011
  * that this driver was written for probably does not have enough RAM to support
  * FreeRTOS (for which such a driver has been written) but versions of the chip
  * expected in 2025 are supposed to have more RAM which may make this possible.
  */

/* Includes */
#include "stm32c0x1_one_wire.h"
#include "stm32c0xx_hal.h"
#include <stdint.h>
#include <stdlib.h>

/* Defines */

/**
 * For the sake of simplicity, the timer resolution will be 1usec.
 * TODO: this should be defined relative to the actual main clock
 * frequency, where this now assumes it is 48MHz.
 */
#define PRESCALER_1_USEC                            48  /* TODO: Assumes 48MHz clock */

/**
 * In general, the timer will be allowed to run until it would overflow.
 * Elapsed time intervals will be detected through output compare.
 */
#define ONE_WIRE_DELAY_TIMER_PERIOD_DEFAULT_USEC          65535

/* The One-Wire specification states that a strobe of >= 480usec
 * is required to indicate a reset.  A little extra time
 * will ensure that it is properly identified.
 */
#define ONE_WIRE_RESET_STROBE_WIDTH_USEC            500

/* Delay from when IO line is released after reset pulse
 * to when trigger becomes sensitive to a presence pulse.
 * Presence pulse could happen as soon as 15usec after rising
 * edge, so this should be large enough to allow for the line
 * to be pulled up, but not so close to 15usec that the falling edge
 * could be missed.
 */
#define ONE_WIRE_PRESENCE_DETECT_SETUP_DELAY_USEC   10

/* Maximum delay when looking for a presence pulse.  If this much
 * time elapses, then no device has been detected.
 */
#define ONE_WIRE_PRESENCE_DETECT_TIMEOUT_USEC       300

#define ONE_WIRE_DELAY_CAPTURE_PERIOD_DEFAULT_USEC  ONE_WIRE_PRESENCE_DETECT_TIMEOUT_USEC

/* How long to leave the IO line high so that the device(s) can
 * refill their energy store
 */
#define ONE_WIRE_RECOVERY_TIME_USEC                 5

/* Extern functions */
extern void Error_Handler(void);

/* Local types */
typedef enum {
    ONE_WIRE_STATE_INIT = 0,
    ONE_WIRE_STATE_IDLE = 1,
    ONE_WIRE_STATE_RESET_STROBE = 2,
    ONE_WIRE_STATE_PRESENCE_CAPTURE = 3,
    ONE_WIRE_STATE_RECOVERY = 4,
    /* TODO: add states related to read and write */
} __one_wire_state_t;

typedef int (*one_wire_hardware_init_fn)(void);
typedef void (*one_wire_hardware_control_pin_as_gpio_fn)(void);
typedef int (*one_wire_hardware_setup_interval_timer_fn)(uint32_t interval_usec);
typedef int (*one_wire_hardware_setup_input_capture_fn)(uint32_t timeout_usec);
typedef void (*one_wire_hardware_start_input_capture_fn)();

typedef struct {
    uint32_t capture_timer_channel;
    uint32_t delay_timer_channel;
    GPIO_TypeDef *gpio;
    uint32_t pin;
    one_wire_hardware_init_fn init;
    one_wire_hardware_control_pin_as_gpio_fn control_pin_as_gpio;
    one_wire_hardware_setup_interval_timer_fn setup_interval_timer;
    one_wire_hardware_setup_input_capture_fn setup_input_capture;
    one_wire_hardware_start_input_capture_fn start_input_capture;
} __one_wire_bus_fixed_t;

typedef struct {
    TIM_HandleTypeDef delay_timer;
    TIM_HandleTypeDef capture_timer;
    __one_wire_state_t state;
    stm32c0x1_one_wire_presence_callback_fn presence_callback;
    one_wire_presence_result_t presence_result;
    /* TODO: this could have parameters such as target data rate (standard
     * versus overdrive) which would affect how the signals are generated.
     * Without these, it is assumed that the standard speed will be used.
     */
} __one_wire_bus_var_t;

typedef struct {
    __one_wire_bus_fixed_t fixed;
    __one_wire_bus_var_t var;
} __one_wire_bus_t;

/* Local function prototypes */
static int __init_bus_0();
static void __setup_bus_0_control_pin_as_gpio();
static int __setup_bus_0_interval_timer(uint32_t interval_usec);
static int __setup_bus_0_input_capture(uint32_t timeout_usec);
static void __start_bus_0_input_capture_timer();

static void __start_reset_pulse(uint8_t bus_index);
static void __finish_reset_pulse(uint8_t bus_index);
static void __setup_input_capture(uint8_t bus_index, uint32_t setup_delay_usec,
    uint32_t timeout_usec);
static void __wait_for_recovery(uint8_t bus_index);
static void __finish_recovery(uint8_t bus_index);
static void __clear_bus_state(uint8_t bus_index);

void __handle_input_capture_event(uint8_t bus_index, uint16_t capture_value);
void __handle_timer_update_event(uint8_t bus_index);

/* File-scoped variables */
static __one_wire_bus_t __buses[] = {
    {   /* Bus index 0 */
        .fixed = {
            .capture_timer_channel = TIM_CHANNEL_4, // TIM1 channel 4
            .delay_timer_channel = TIM_CHANNEL_1,   // TIM14 channel 1
            .gpio = GPIOA,
            .pin = GPIO_PIN_3,
            .init = __init_bus_0,
            .control_pin_as_gpio = __setup_bus_0_control_pin_as_gpio,
            .setup_interval_timer = __setup_bus_0_interval_timer,
            .setup_input_capture = __setup_bus_0_input_capture,
            .start_input_capture = __start_bus_0_input_capture_timer,
        },
        .var = {
            .delay_timer = {
                .Instance = TIM14,
                .Init = {
                    .Prescaler = PRESCALER_1_USEC,
                    .CounterMode = TIM_COUNTERMODE_UP,
                    .Period = ONE_WIRE_DELAY_TIMER_PERIOD_DEFAULT_USEC,
                    .ClockDivision = TIM_CLOCKDIVISION_DIV1,
                    .RepetitionCounter = 0,
                    .AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE,
                },
            },
            .capture_timer = {
                .Instance = TIM1,
                .Init = {
                    .Prescaler = PRESCALER_1_USEC,
                    .CounterMode = TIM_COUNTERMODE_UP,
                    .Period = ONE_WIRE_DELAY_CAPTURE_PERIOD_DEFAULT_USEC,
                    .ClockDivision = TIM_CLOCKDIVISION_DIV1,
                    .RepetitionCounter = 0,
                    .AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE,
                },
            },
            .state = ONE_WIRE_STATE_INIT,
            .presence_callback = NULL,
            .presence_result = ONE_WIRE_PRESENCE_UNDEFINED,
        },
    },
    /* Other buses could be defined here if they exist on the platform */
};

static uint8_t __bus_count = sizeof(__buses)/sizeof(__one_wire_bus_t);

int stm32c0x1_one_wire_initialize() {
    /* TODO: not sure if this needs to abort any actions already in
     * progress.
     */
    
    return 0;   /* Success */
}

uint8_t stm32c0x1_one_wire_get_bus_count() {
    return __bus_count;
}

int stm32c0x1_one_wire_initialize_bus(uint8_t bus_index) {
    int result;
    
    if(bus_index >= __bus_count) {
        return -1;  /* Invalid bus index */
    }
    
    __clear_bus_state(bus_index);   /* TODO: Should this just abort instead? */
    
    if(NULL != __buses[bus_index].fixed.init) {
        /* Call the peripheral-specific init function */
        result = __buses[bus_index].fixed.init();
        
        if(0 != result) {
            return result;
        }
    }
    
    /* TODO: fill this in as needed */
    
    return 0;   /* Success */
}

void stm32c0x1_one_wire_abort(uint8_t bus_index) {
    /* TODO: shut down the timer peripheral, release the pin, and
     * reset the state for this bus.
     */
}

/* The one-wire protocol has a single data line (IO) that is pulled up by a resistor.
 * To signal a reset, the master pulls IO low for at least 480 usec and then releases
 * it.  (Note that this is for standard speed.  Overdrive mode, which increases the
 * data rate, uses shorter delays.  Support could be added but this is not
 * a priority for the initial version.)
 *
 * After this reset strobe, one or more devices on the bus can indicate their
 * presence by pulling the IO line low between 60-75usec later and then releasing it.
 * A recovery period (during which the device recharges its internal energy store)
 * follows this prior to any further communication.
 */
int stm32c0x1_one_wire_reset_and_detect_presence(uint8_t bus_index,
        stm32c0x1_one_wire_presence_callback_fn callback) {
    if(bus_index >= __bus_count) {
        return -1;  /* Invalid bus index */
    }
    
    if(ONE_WIRE_STATE_IDLE != __buses[bus_index].var.state) {
        return -2;  /* Invalid state, some other signal already in progress */
    }
    
    /* Cache state that will be needed to handle the end-of-strobe interrupt */
    __buses[bus_index].var.state = ONE_WIRE_STATE_RESET_STROBE;
    __buses[bus_index].var.presence_callback = callback;
    
    /* Start reset strobe and configure timer to generate an interrupt
     * after a delay.  The interrupt handler invoked at the end of
     * the delay will be used to start the presence detect sequence,
     * and a subsequent interrupt will cause the callback to be called
     * with the detection result.
     */
    __start_reset_pulse(bus_index);
    
    return 0;       /* Reset started but will take some time to complete */
}

void __clear_bus_state(uint8_t bus_index) {
    __buses[bus_index].var.state = ONE_WIRE_STATE_IDLE;
    __buses[bus_index].var.presence_callback = NULL;
    __buses[bus_index].var.presence_result = ONE_WIRE_PRESENCE_UNDEFINED;
}

void __start_reset_pulse(uint8_t bus_index) {
    /* Pull IO line low for >= 480usec and then trigger an interrupt */
    
    /* Get the timer configured but do not start it yet */
    __buses[bus_index].fixed.setup_interval_timer(ONE_WIRE_RESET_STROBE_WIDTH_USEC);
    
    /* Drive the pin low */
    __buses[bus_index].fixed.control_pin_as_gpio();
    HAL_GPIO_WritePin(__buses[bus_index].fixed.gpio,
        __buses[bus_index].fixed.pin, GPIO_PIN_RESET);
    
    /* Interval timer is technically in output compare mode even though it has
     * no output configured.
     */
    HAL_TIM_OC_Start_IT(&__buses[bus_index].var.delay_timer,
        __buses[bus_index].fixed.delay_timer_channel);
}

void __finish_reset_pulse(uint8_t bus_index) {
    /* Release the pin */
    HAL_GPIO_WritePin(__buses[bus_index].fixed.gpio,
        __buses[bus_index].fixed.pin, GPIO_PIN_SET);
    
    __buses[bus_index].var.state = ONE_WIRE_STATE_PRESENCE_CAPTURE;
    
    /* Configure interrupts to occur on either the next falling edge of
     * the IO line, or a timeout
     */
    __setup_input_capture(bus_index, ONE_WIRE_PRESENCE_DETECT_SETUP_DELAY_USEC,
        ONE_WIRE_PRESENCE_DETECT_TIMEOUT_USEC);
}

void __setup_input_capture(uint8_t bus_index, uint32_t setup_delay_usec,
        uint32_t timeout_usec) {
    
    /* Become sensitive to falling edge and either interrupt when falling edge
     * occurs or when timeout is reached.
     */
    __buses[bus_index].fixed.setup_input_capture(timeout_usec);

    __HAL_TIM_CLEAR_FLAG(&__buses[bus_index].var.capture_timer, TIM_FLAG_UPDATE);
    
    /* Start the timer */
    __buses[bus_index].fixed.start_input_capture();

    /* TODO: remove debug code */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    int temp = 0;
    while(temp++ < 100)
            ;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
}

void __wait_for_recovery(uint8_t bus_index) {
    /* Note that this leaves the pin in its previous state */
    
    /* FIXME: recovery should technically start after the end of
     * any slave device activity, i.e. when it releases the data
     * line and it is pulled high.  The current state machine
     * causes the recovery pattern to start when the detect pulse
     * falling edge is captured, instead of when the rising edge
     * happens, so it is premature.  A simple way to fix this would
     * be to extend the recovery period well past when the slave
     * would release the line.  The more correct way to fix this
     * is to detect the rising edge and start the recovery at that point.
     * The presence pulse should be long enough to allow this edge
     * to be captured.  The latter approach would allow any
     * subsequent transactions to be started immediately instead of
     * after a conservative timeout.
     */
    
    /* Get the timer configured but do not start it yet */
    __buses[bus_index].fixed.setup_interval_timer(ONE_WIRE_RECOVERY_TIME_USEC);
    
    __buses[bus_index].var.state = ONE_WIRE_STATE_RECOVERY;
    
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); /* TODO: remove debug code */
    
    /* Interval timer is technically in output compare mode even though it has
     * no output configured.
     */
    HAL_TIM_OC_Start_IT(&__buses[bus_index].var.delay_timer,
        __buses[bus_index].fixed.delay_timer_channel);
}

void __finish_recovery(uint8_t bus_index) {
    /* This function will invoke a user callback if one was provided.
     * The callback might attempt to command the bus to do something new,
     * so its state needs to be ready for that.  This caches the
     * callback information, clears the state so new commands can be
     * accepted, and then invokes the callback.
     *
     * Another approach would be to have an interrupt handler at a lower
     * priority that is triggered in software, and allows this to gracefully
     * end before the callback happens, but that may be unnecessarily complex.
     */
    stm32c0x1_one_wire_presence_callback_fn presence_callback;
    one_wire_presence_result_t presence_result;
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); /* TODO: remove debug code */
    
    /* Cache the callback and result */
    presence_callback = __buses[bus_index].var.presence_callback;
    presence_result = __buses[bus_index].var.presence_result;
    
    /* Reset to the idle state */
    __clear_bus_state(bus_index);
    
    if(NULL != presence_callback) {
        /* Assume that the callback may cause a new bus request to
         * be made before this returns.
         */
        presence_callback(presence_result);
    }
}

void __handle_input_capture_event(uint8_t bus_index, uint16_t capture_value) {
    /* The timer has captured an edge at the indicated timestamp */
    if(ONE_WIRE_STATE_PRESENCE_CAPTURE == __buses[bus_index].var.state) {
        /* TODO: could do some sanity checking on the timestamp here and call it
         * not detected if it was out of bounds
         */
        /* Stop the timer.  This also shuts down the peripheral but that should not cause
         * any issues.
         */
        HAL_TIM_IC_Stop_IT(&__buses[bus_index].var.capture_timer,
            __buses[bus_index].fixed.capture_timer_channel);
        __buses[bus_index].var.presence_result = ONE_WIRE_PRESENCE_DETECTED;
        __wait_for_recovery(bus_index);
    } else {
        /* Receiving an event in the wrong state could indicate a bug, or it could
         * be a race condition (e.g. edge was detected just as time ran out).
         * A trap could be added here to help figure out unexpected behavior.
         */
    }
}

void __handle_timer_expired_event(uint8_t bus_index) {
    /* Input capture timer has reached overflow and restarted, or a value match
     * indicates the end of a delay
     */
    
    if(ONE_WIRE_STATE_RESET_STROBE == __buses[bus_index].var.state) {
        /* Normal end to the reset strobe */
        /* Stop the timer */
        HAL_TIM_OC_Stop_IT(&__buses[bus_index].var.delay_timer,
            __buses[bus_index].fixed.delay_timer_channel);
        __finish_reset_pulse(bus_index);
    } else if(ONE_WIRE_STATE_PRESENCE_CAPTURE == __buses[bus_index].var.state) {
        /* The timeout elapsed before a falling edge was detected on input */
        /* Stop the timer */
        HAL_TIM_IC_Stop_IT(&__buses[bus_index].var.capture_timer,
            __buses[bus_index].fixed.capture_timer_channel);
        __buses[bus_index].var.presence_result = ONE_WIRE_PRESENCE_NOT_DETECTED;
        __wait_for_recovery(bus_index);
    } else if(ONE_WIRE_STATE_RECOVERY == __buses[bus_index].var.state) {
        /* Normal end to the recovery period */
        /* Stop the timer */
        HAL_TIM_OC_Stop_IT(&__buses[bus_index].var.delay_timer,
            __buses[bus_index].fixed.delay_timer_channel);
        __finish_recovery(bus_index);
    }
}

void __setup_bus_0_control_pin_as_gpio(void) {
    /* NOTE: these settings were originally created by the STM32CubeMX code generator
     * and then heavily reworked.  The code generator would initialize the configuration
     * structs to zero and then individually assign the fields to constants.  This
     * initializes the structs to their final values so this can be done at compile
     * time and reduce setup latency.  Any unused fields still have to be zeroed out.
     */

    /* Set up GPIO pin PA3 as open-drain output */
    GPIO_InitTypeDef GPIO_InitStruct = {
        .Pin = GPIO_PIN_3,     /* TODO: PA3 is hardcoded */
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = 0,
    };

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void __setup_bus_0_control_pin_with_timer() {
    uint32_t tmp;
    
    /* This had to be significantly optimized to allow the falling edge to be caught in time */
    
    /* Set up alt function.  Each pin gets 4 bits in AFR, so to refer to
     * a particular pin rank, shift 4*rank
     */
    tmp = GPIOA->AFR[0];
    tmp &= ~(0xFUL << 12U);                 /* Clear bits for AFSEL3 (PA3 alt function) */
    tmp |= (GPIO_AF5_TIM1 & 0x0FUL) << 12U; /* Set bits for AFSEL3 (PA3 alt function) */
    GPIOA->AFR[0] = tmp;
    
    /* Set up mode.  Each pin gets 2 bits in MODER, so to refer to
     * a particular pin rank, shift 2*rank
     */
    tmp = GPIOA->MODER;
    tmp &= ~(GPIO_MODER_MODE0 << 6U);       /* Clear bits for MODE3 (PA3 mode) */
    tmp |= (GPIO_MODE_AF_OD & 3U) << 6U;    /* Set bits for MODE3 (PA3 mode) */
    GPIOA->MODER = tmp;
    
    /* Set up speed.  Each pin gets 2 bits in OSPEEDR, so to refer to
     * a particular pin rank, shift 2*rank */
    tmp = GPIOA->OSPEEDR;
    tmp &= ~(GPIO_OSPEEDR_OSPEED0 << 6U);   /* Clear bits for OSPEED3 (PA3 output speed) */
    tmp |= (GPIO_SPEED_FREQ_LOW << 6U);     /* Set bits for OSPEED3 (PA3 output speed) */
    GPIOA->OSPEEDR = tmp;
    
    /* Set up output type.  Each pin gets 1 bit in OTYPER. */
    tmp = GPIOA->OTYPER;
    tmp &= ~(GPIO_OTYPER_OT0 << 3) ;        /* Clear bit for OT3 (PA3 PP vs. OD) */
    tmp |= (((GPIO_MODE_AF_OD & 0x10U) >> 4U) << 3); /* Set bit for OT3 (PA3 open drain) */
    GPIOA->OTYPER = tmp;
}

void __start_bus_0_input_capture_timer() {
    /* This has been heavily optimized to avoid missing the falling edge */
    
    /* Reset timer counter */
    TIM1->CNT = 0;
    
    /* Enable interrupt source */
    TIM1->DIER |= TIM_IT_CC4;       /* TIM1_CH4 */
    
    /* Enable the capture channel.  There are four bits per timer channel. */
    TIM1->CCER |= (1 << 12);        /* Bit 12 is CC4E (channel 4 enable) */
    
    /* Enable the timer */
    TIM1->CR1 |= TIM_CR1_CEN;
}

int __init_bus_0() {
    /* This driver requires that the pin control change modes several
     * times throughout a transaction.  This sets up the idle state, and other
     * functions are responsible for altering the configuration at appropriate times.
     */
    
    /* NOTE: most of this was originally created by the STM32CubeMX code generator */
    
    TIM_MasterConfigTypeDef sMasterConfig = {
        .MasterOutputTrigger = TIM_TRGO_RESET,
        .MasterOutputTrigger2 = TIM_TRGO2_RESET,
        .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE,
    };
    TIM_IC_InitTypeDef sConfigIC = {
        /* Input capture needs to be sensitive to the falling edge only */
        .ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING,
        .ICSelection = TIM_ICSELECTION_DIRECTTI,
        .ICPrescaler = TIM_ICPSC_DIV1,
        .ICFilter = 3, /* Requires 8 consecutive samples at new level to trigger */
    };
    
    /* Peripheral clock enables */
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_TIM14_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* Configure GPIO pin Output Level to high */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);     /* TODO: PA3 is hardcoded */
    
    /* Put the pin into GPIO mode.  This will cause it to idle high. */
    __setup_bus_0_control_pin_as_gpio();
    
    /* Initialize delay timer (TIM14) */
    if (HAL_TIM_OnePulse_Init(&__buses[0].var.delay_timer, TIM_OPMODE_SINGLE) != HAL_OK) {
        Error_Handler();
    }
    
    /* Initialize input capture timer (TIM1) */
    if (HAL_TIM_IC_Init(&__buses[0].var.capture_timer) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIMEx_MasterConfigSynchronization(&__buses[0].var.capture_timer, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_IC_ConfigChannel(&__buses[0].var.capture_timer, &sConfigIC,
            __buses[0].fixed.capture_timer_channel) != HAL_OK) {
        Error_Handler();
    }
    
    /* Update event only occurs for overflow/underflow, not setting UG bit */
    __buses[0].var.capture_timer.Instance->CR1 |= TIM_CR1_URS;
    __buses[0].var.delay_timer.Instance->CR1 |= TIM_CR1_URS;
    
    /* Enable interrupt on timer overflow */
    __buses[0].var.capture_timer.Instance->DIER |= TIM_IT_UPDATE;
    
    /* Timer automatically stops when it reaches the end.
     * This is to aid in debugging.
     */
    //__buses[0].var.capture_timer.Instance->CR1 |= TIM_CR1_OPM;
    
    HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0, 0);
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0, 0);
    
    return 0;   /* Success */
}

int __setup_bus_0_interval_timer(uint32_t interval_usec) {
    /* NOTE: these settings were originally created by the STM32CubeMX code generator
     * and then heavily reworked.  The code generator would initialize the configuration
     * structs to zero and then individually assign the fields to constants.  This
     * initializes the structs to their final values so this can be done at compile
     * time and reduce setup latency.  Any unused fields still have to be zeroed out.
     */

    /* NOTE: this function does not change the pin to be under GPIO control.
     * Responsibility for that is with the caller since multiple intervals might be
     * timed without needing to change the pin mode in between.
     */
    
    TIM_OC_InitTypeDef sConfigOC = {
        .OCMode = TIM_OCMODE_TIMING,
        .Pulse = interval_usec,
        .OCPolarity = TIM_OCPOLARITY_HIGH,
        .OCNPolarity = TIM_OCNPOLARITY_HIGH,
        .OCFastMode = TIM_OCFAST_DISABLE,
        .OCIdleState = TIM_OCIDLESTATE_RESET,
        .OCNIdleState = TIM_OCNIDLESTATE_RESET,
    };

    /* Disable interrupts that might be caused by running timer */
    HAL_NVIC_DisableIRQ(TIM14_IRQn);
    
    /* Stop timer if needed */
    __buses[0].var.delay_timer.Instance->CR1 &= ~(TIM_CR1_CEN);
    
    /* Clear any pending events */
    __HAL_TIM_CLEAR_FLAG(&__buses[0].var.delay_timer, TIM_FLAG_CC1);
    
    /* Reset the timer count to zero */
    __buses[0].var.delay_timer.Instance->CNT = 0;
    
    /* Note that the only thing that should need to be updated is the pulse delay. */

    if (HAL_TIM_OC_ConfigChannel(&__buses[0].var.delay_timer, &sConfigOC,
            __buses[0].fixed.delay_timer_channel) != HAL_OK) {
        Error_Handler();
    }
    
    /* Enable interrupt.  Note that the timer still needs to be started. */
    HAL_NVIC_EnableIRQ(TIM14_IRQn);

    return 0;
}

int __setup_bus_0_input_capture(uint32_t timeout_usec) {
    /* NOTE: these settings were originally created by the STM32CubeMX code generator
     * and then heavily reworked.  The code generator would initialize the configuration
     * structs to zero and then individually assign the fields to constants.  This
     * initializes the structs to their final values so this can be done at compile
     * time and reduce setup latency.  Any unused fields still have to be zeroed out.
     */
    
    /* NOTE: This function does configure the pin to be in timer mode since that is
     * a fundamental prerequisite of the input capture setting.
     */
    
    /* Disable interrupts that might be caused by running timer */
    HAL_NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
    HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
    
    /* Stop timer if needed */
    __buses[0].var.capture_timer.Instance->CR1 &= ~(TIM_CR1_CEN);
    
    /* Clear any pending events */
    __HAL_TIM_CLEAR_FLAG(&__buses[0].var.capture_timer, TIM_FLAG_CC4);     // TODO: TIM1_CH4 is hardcoded
  
    /* Set up GPIO pin as a timer pin */
    __setup_bus_0_control_pin_with_timer();

    /* TIM1 interrupt Init */
    HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
    
    /* Note that this does not start the timer running */
    
    return 0;
}

void TIM1_CC_IRQHandler(void) {
    /* This is the handler for TIM1 compare and capture events */
    
    /* TODO: the association of TIM1 channel 2 to __buses[0] could be
     * made explicit by having this search through __buses,
     * but until there is more than one bus, this isn't critical.
     */
    
    uint16_t capture_value;
    
    if(TIM_FLAG_CC4 == (TIM1->SR & TIM_FLAG_CC4)) {     // TODO: TIM1_CH4 is hardcoded
        /* Channel 2 has performed an input capture */
        /* NOTE: the HAL macro calls this register COMPARE but it is the CAPTURE/COMPARE register
         * whose behavior depends on whether the channel is configured as input or output.
         */
        /* Disable the interrupt in case the signal is glitchy.  We only care about one falling edge. */
        HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
        
        /* Clear the event flag */
        TIM1->SR &= ~(TIM_FLAG_CC4);     // TODO: TIM1_CH4 is hardcoded
        if(ONE_WIRE_STATE_PRESENCE_CAPTURE == __buses[0].var.state) {
            /* Read out the captured interval and call handler */
            capture_value = __HAL_TIM_GET_COMPARE(&__buses[0].var.capture_timer,
                __buses[0].fixed.capture_timer_channel);
            __handle_input_capture_event(0, capture_value);
        } else {
            /* This timer channel is only used for input capture.  If the interrupt was
             * received in any other state, it is an error.  The flag was cleared
             * with interrupts disabled on setup, so this should never happen.
             */
            Error_Handler();
        }
    }
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    /* This is the handler for TIM1 update, break, and commutation */
    
    /* TODO: the association of TIM1 channel 2 to __buses[0] capture timer could be
     * made explicit by having this search through __buses,
     * but until there is more than one bus, this isn't critical.
     */
    
    if(__HAL_TIM_GET_FLAG(&__buses[0].var.capture_timer, TIM_FLAG_UPDATE)) {
        __HAL_TIM_CLEAR_FLAG(&__buses[0].var.capture_timer, TIM_FLAG_UPDATE);
        /* During debugging, this may be invoked if the timer is allowed to
         * free-run such that it expires before the desired event can occur
         */
        if(ONE_WIRE_STATE_PRESENCE_CAPTURE == __buses[0].var.state) {
            /* Timer interval expired during input capture */
            __handle_timer_expired_event(0);
        } else {
            /* This could potentially happen if the input capture occurred just as
             * the interval expired, especially if the relative IRQ priorities
             * weren't right, or due to debugging.  Hitting this case isn't
             * really desirable, but might happen for innocuous reasons.
             */
        }
    }
}

void TIM14_IRQHandler(void) {
    /* TODO: the association of TIM14 channel 1 to __buses[0] delay timer could be
     * made explicit by having this search through __buses,
     * but until there is more than one bus, this isn't critical.
     */
    if(__HAL_TIM_GET_FLAG(&__buses[0].var.delay_timer, TIM_FLAG_CC1)) {
        /* Disable interrupt -- all use cases are one-shots. */
        HAL_NVIC_DisableIRQ(TIM14_IRQn);
        
        /* Clear the event flag */
        __HAL_TIM_CLEAR_FLAG(&__buses[0].var.delay_timer, TIM_FLAG_CC1);
        
        if(ONE_WIRE_STATE_PRESENCE_CAPTURE == __buses[0].var.state) {
            /* Input capture and delay timer should never be active at once */
            Error_Handler();
        } else {
            /* This event will occur whenever the interval timer expires, but is
             * not used for input capture.
             */
            __handle_timer_expired_event(0);
        }
    }
}
