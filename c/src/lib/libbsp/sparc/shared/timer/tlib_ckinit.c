/*
 *  Clock Tick Device Driver using Timer Library implemented
 *  by the GRLIB GPTIMER / LEON2 Timer drivers.
 *
 *  COPYRIGHT (c) 2010.
 *  Aeroflex Gaisler AB.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 */

#include <stdlib.h>
#include <bsp.h>
#include <tlib.h>

/* Undefine (default) this to save space in standard LEON configurations,
 * it will assume that Prescaler is running at 1MHz.
 */
/*#undef CLOCK_DRIVER_DONT_ASSUME_PRESCALER_1MHZ*/

/* Set the below defines from bsp.h if function needed.
#undef CLOCK_DRIVER_ISRS_PER_TICK
#undef CLOCK_DRIVER_USE_FAST_IDLE
*/
#define Clock_driver_support_at_tick()

/*
 *  Number of Clock ticks since initialization
 */
volatile uint32_t Clock_driver_ticks;

/*
 *  Timer Number in Timer Library. Defaults to the first Timer in
 *  the System.
 */
int Clock_timer = 0;

/*
 * Timer Handle in Timer Library
 */
void *Clock_handle = NULL;

#ifdef CLOCK_DRIVER_DONT_ASSUME_PRESCALER_1MHZ
unsigned int Clock_basefreq;
#endif

void Clock_exit(void);

/*
 *  Major and minor number.
 */

rtems_device_major_number rtems_clock_major = UINT32_MAX;
rtems_device_minor_number rtems_clock_minor;

/*
 *  Clock_isr
 *
 *  This is the clock tick interrupt handler.
 *
 *  Input parameters:
 *    vector - vector number
 *
 *  Output parameters:  NONE
 *
 *  Return values:      NONE
 *
 */

void Clock_isr(void *arg_unused)
{
  /*
   *  Accurate count of ISRs
   */

  Clock_driver_ticks += 1;

#ifdef CLOCK_DRIVER_USE_FAST_IDLE
  do {
    rtems_clock_tick();
  } while ( _Thread_Executing == _Thread_Idle &&
          _Thread_Heir == _Thread_Executing);

  Clock_driver_support_at_tick();
  return;

#else

  /* 
   * Add custom handling at every tick from bsp.h
   */
  Clock_driver_support_at_tick();

#ifdef CLOCK_DRIVER_ISRS_PER_TICK
  /*
   *  The driver is multiple ISRs per clock tick.
   */

  if ( !Clock_driver_isrs ) {

    rtems_clock_tick();

    Clock_driver_isrs = CLOCK_DRIVER_ISRS_PER_TICK;
  }
  Clock_driver_isrs--;
#else

  /*
   *  The driver is one ISR per clock tick.
   */
  rtems_clock_tick();
#endif
#endif
}

/*
 *  Clock_exit
 *
 *  This routine allows the clock driver to exit by masking the interrupt and
 *  disabling the clock's counter.
 *
 *  Input parameters:   NONE
 *
 *  Output parameters:  NONE
 *
 *  Return values:      NONE
 *
 */

void Clock_exit( void )
{
  /* Stop all activity of the Timer, no more ISRs.
   * We could be using tlib_close(), however tlib_stop() is quicker
   * and independent of IRQ unregister code.
   */
  if ( Clock_handle ) {
    tlib_stop(Clock_handle);
    Clock_handle = NULL;
  }
}

uint32_t Clock_nanoseconds_since_last_tick(void)
{
  uint32_t clicks;
  if ( !Clock_handle )
    return 0;

  tlib_get_counter(Clock_handle, (unsigned int *)&clicks);

#ifdef CLOCK_DRIVER_DONT_ASSUME_PRESCALER_1MHZ
  {
    /* Down counter. Calc from BaseFreq. */
    uint64_t tmp;
    tmp = ((uint64_t)clicks * 1000000000) / ((uint64_t)Clock_basefreq);
    return (uint32_t)tmp;
  }
#else
  /* Down counter. Timer base frequency is initialized to 1 MHz */
  return (uint32_t)
     (rtems_configuration_get_microseconds_per_tick() - clicks) * 1000;
#endif
}

/*
 *  Clock_initialize
 *
 *  This routine initializes the clock driver and starts the Clock.
 *
 *  Input parameters:
 *    major - clock device major number
 *    minor - clock device minor number
 *    parg  - pointer to optional device driver arguments
 *
 *  Output parameters:  NONE
 *
 *  Return values:
 *    rtems_device_driver status code
 */

rtems_device_driver Clock_initialize(
  rtems_device_major_number major,
  rtems_device_minor_number minor,
  void *pargp
)
{
  unsigned int tick_hz;

  /*
   *  Take Timer that should be used as system timer.
   *
   */
  Clock_handle = tlib_open(Clock_timer);
  if ( Clock_handle == NULL ) {
#warning IMPLEMENT ERROR HANDLING HERE
    return;
  }

  /*
   *  Install Clock ISR before starting timer
   */
  tlib_irq_register(Clock_handle, Clock_isr, NULL);

  /* Set Timer Frequency to tick at Configured value. The Timer
   * Frequency is set in multiples of the timer base frequency.
   *
   * In standard LEON3 designs the base frequency is is 1MHz, to
   * save instructions define CLOCK_DRIVER_ASSUME_PRESCALER_1MHZ
   * to avoid 64-bit calculation.
   */
#ifdef CLOCK_DRIVER_DONT_ASSUME_PRESCALER_1MHZ
  {
    uint64_t tmp;

    tlib_get_freq(Clock_handle, &Clock_basefreq, NULL);

    tmp = (uint64_t)Clock_basefreq;
    tmp = tmp * (unint64_t)rtems_configuration_get_microseconds_per_tick();
    tick_hz = tmp / 1000000;
  }
#else
  tick_hz = rtems_configuration_get_microseconds_per_tick();
#endif

  tlib_set_freq(Clock_handle, tick_hz);

  rtems_clock_set_nanoseconds_extension(Clock_nanoseconds_since_last_tick);

  /*
   *  IRQ and Frequency is setup, now we start the Timer. IRQ is still
   *  disabled globally during startup, so IRQ will hold for a while.
   */
  tlib_start(Clock_handle, 0);

  /*
   *  Register function called at system shutdown
   */
  atexit( Clock_exit ); 

  /*
   * make major/minor avail to others such as shared memory driver
   */

  rtems_clock_major = major;
  rtems_clock_minor = minor;

  /*
   *  If we are counting ISRs per tick, then initialize the counter.
   */

#ifdef CLOCK_DRIVER_ISRS_PER_TICK
  Clock_driver_isrs = CLOCK_DRIVER_ISRS_PER_TICK;
#endif

  return RTEMS_SUCCESSFUL;
}

/*** Timer Driver Interface ***/

void Clock_timer_register(int timer_number)
{
  Clock_timer = timer_number;
}
