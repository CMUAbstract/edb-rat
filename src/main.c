#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <libmsp/periph.h>
#include <libmsp/clock.h>
#include <libmsp/watchdog.h>
#include <libmsp/gpio.h>

#ifdef CONFIG_EDB
#include <libedb/edb.h>
#endif

#include "pins.h"

static void init_hw()
{
    msp_watchdog_disable();
    msp_gpio_unlock();
    msp_clock_setup();
}

int main() {
    init_hw();

#ifdef CONFIG_EDB
    edb_init();
#endif

    while(1) {

#ifdef TEST_WATCHPOINTS
        WATCHPOINT(0);
        for (unsigned i = 0; i < 100; ++i);

        WATCHPOINT(1);
        for (unsigned i = 0; i < 100; ++i);

        WATCHPOINT(1);
        for (unsigned i = 0; i < 100; ++i);
#endif // TEST_WATCHPOINTS

#ifdef TEST_BREAKPOINTS
        EXTERNAL_BREAKPOINT(0);
        for (unsigned i = 0; i < 500; ++i);

        EXTERNAL_BREAKPOINT(1);
        for (unsigned i = 0; i < 500; ++i);
#endif // TEST_BREAKPOINTS

#ifdef TEST_ENERGY_GUARDS
        WATCHPOINT(0);

        ENERGY_GUARD_BEGIN();
        for (unsigned i = 0; i < 5000; ++i);
        ENERGY_GUARD_END();

        WATCHPOINT(1);

        for (unsigned i = 0; i < 500; ++i);
#endif // TEST_ENERGY_GUARDS

    }

    return 0;
}
