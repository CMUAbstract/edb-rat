edb-rat: an app for testing EDB features
========================================

This repository contains a "lab rat" test application on which we will
demonstrate features of EDB. This repository also contains "links" in the form
of git submodules to a frozen version of the EDB firmware and the EDB console
repos.

Below is a quick start tutorial for how to use EDB to debug an application.
The application runs on a WISP5 energy-harvesting device.

## Prep

This repository also includes as submodules the EDB firmware and host console
as an external tool dependency (in `ext/`) -- this is done to make the
procedure as reproducible as possible, submodules freeze versions of
interdependent components.

To start, clone this repository with `git clone --recursive ...`, in order
to fetch it along with the submodules.

The app (and most of its dependencies) are built using the `maker` build
framework. The `maker` repos is also included as a submodule in `ext/`.

### Optional: build and flash EDB firmware

If you have a board, chances are the firware has already been flashed onto it,
but if needed you can always build and flash the firmware as follows:

    cd ext/edb-server
    make bld/gcc/all
    make bld/gcc/prog
    cd ..

The edb-rat application has a separate built configuration for testing each
feature. To select the build variant, set *one of the* `TEST_*` variables to
1 in `bld/Makefile`.

### Build and flash edb-rat app

To build and flash the edb-rat app onto the WISP, run:

    make bld/gcc/clean
    make bld/gcc/all
    make bld/gcc/prog

NOTE: After changing the build configuration, make sure to clean.
NOTE: After cleaning, it is sufficient to make the `bld/gcc/prog` target,
since `bld/gcc/all` is a prerequisite.

Connect the EDB to the WISP by connecting the two ends of the matching
8+3-pin board-to-board headers. Then, separately, connect three additional
wires: the Vcap line, RF RX line and RF TX line. On EDBv1.1 board, these
connections go to the 5-pin header located near the programming header
on the bottom side of the board. On EDBv1.0 board, these connections
go to separate test points in the upper region on the board.

Open the EDB console and attach to the debugger, specifying the TTY
device that shows up when you plug in the board (see tips below):

    cd ext/edb-console
    ./edb
    > attach --device /dev/ttyUSB0
    >

There should be no need to re-run the attach command as long as the console
stays open, even if the EDB board is reset.

### Tips

To verify that the workstation recognized the EDB board as a USB device, check
that a TTY device was created after the cable was plugged in: 

        ls /dev/ttyUSB*

If you don't see a `/dev/ttyUSB*` device, then check your connections, and check
kernel log (`dmesg`) for any hints, also check lsusb output, which should have:

    Bus 002 Device 023: ID 0403:6001 Future Technology Devices International, Ltd FT232 USB-Serial (UART) IC

On Linux, to grant your user permission to access the device, add it to
`dialout` group:

    sudo usermod -a -G dialout $(whoami)

Linux kernel shipps with the drivers, so no need to install anything. On OS X
driver installation is likely necessary, as described in this [tutorial from
SparkFun](https://learn.sparkfun.com/tutorials/how-to-install-ftdi-drivers/mac).

## Board status

The four LEDs on the top of the board indicate status as follows:

    * GREEN (flashing): device powered (from USB or FET) and idle
    * YELLOW: flashes once when EDB boots
    * ORANGE: on while in interactive debug mode
    * RED: error occured, error code encoded as frequency of flashing (see on scope)

## EDB commands

The `help` command lists available commands. A brief help for each console
command can be obtained by running the command with `--help` argument.

**ATTENTION**: Disconnect WISP from the FET programmer when working with EDB,
because the FET leaks power to the WISP, even when the FET is connected in the
self-powered configuration. Also, when working with the RFID reader, keep it
off while configuring things in the EDB console, and only turn it on once all
is ready for the experiment. Otherwise, events will happen in the middle of
you trying to set things up (e.g. enable watchpoints, etc.), which is very
likely to confuse EDB.

**NOTE**: Before each of the different tests described below, do reset
the EDB board using the on-board reset push-button. This is not always
necessary, but eliminates potential problems.

### Sense and set energy level on the target device

Sense voltage on the target capacitor

    > sense vcap
    Vcap = 2.0121

The next operation to try is to charge and discharge the WISP capacitor:

    > charge 2.2
    Vcap = 2.2001
    > discharge 0.1
    Vcap = 0.1006

The printed Vcap value is the capacitor voltage measured at the completion of
the charging/discharging procedure.

If the command never completes, one possible reason is that the wire to the
WISP capacitor is not connected.

Discharging to precisely 0.0 might not work (edge-case), so use a small
positive value.

### Passively monitor energy, program, and I/O events

#### Stream energy level

Monitor the voltage level on the WISP capacitor using the `stream` command.
[Turn on the reader](#sllurp-toolbox-for-rfid-reader) to see the voltage rising up to 2.4V
(turn-on threshold) as the capacitor charges and very quickly falling to 1.8V
(brown-out threshold) as WISP is actively computing.

    > stream VCAP
    adc_sampling_period_cycles= 3000
    timestamp_sec,VCAP
    0.001013,2.327659
    0.002014,2.332031
    0.003014,2.332031
    0.004014,2.337861
    0.005015,2.342963
    0.006015,2.353165
    0.007015,2.343691
    0.008016,2.331302
    0.009016,2.307982

Ctrl-C ends the stream operation. Note: cleanup bugs may exist, if commands not
working after having run some commands, reset the debugger board and start
fresh.

#### Stream watchpoints

Build the app with `TEST_WATCHPOINTS = 1` and the rest set to 0.

In the EDB console, enable the watchpoints and start monitoring for them using
the `stream` command:

    > watch --energy 0 E
    > watch --energy 1 E
    > stream watchpoints
    adc_sampling_period_cycles=3000
    host_timestamp_sec,timestamp_sec,watchpoint_id,watchpoint_vcap
    13.678827,0.018393,0,2.3517
    13.678827,0.018469,1,2.3466
    13.678827,0.018546,1,2.3408
    13.678827,0.018623,0,2.3451
    13.678827,0.018699,1,2.3393
    13.678827,0.018776,1,2.3313
    13.678827,0.018853,0,2.3335

The `--energy` flag tells EDB to collect the energy level when watchpoint hits.

#### Stream energy-interference-free printf output

The energy-interference-free printf allows a target application to generate
output while running on an intermittent power supply. The output can consume an
arbitrary amount of energy -- EDB compensates.

Build the app with `TEST_EIF_PRINTF = 1` and the rest set to 0.

The application code contains printf or log statements with values
of interest:

    EIF_PRINTF("EDB-RAT\r\n");

In the EDB console, expect the stdout data using the `wait` command:

    > wait
    20.119: EDB-RAT

#### Stream RFID messages

For applications that communicate with the reader over RFID protocol, EDB can
decode the incoming and (partially) outgoing messages (TODO: add to the edb-rat app).

    > attach
    > stream RF_EVENTS
    adc_sampling_period_cycles= 3000
    timestamp_sec,rf_event
    ...

##### Tips

Make sure that the RX and TX lines (not part of the main 8+3 header) are
connected.

Watch output of [SLLURP tool](#sllurp-toolbox-for-rfid-reader) tool to make
sure that communication is actually taking place.

Events are buffered, so it make take quite a few minutes before
they show up in the console.

***NOTE***: This has not been tested in a while, so due to bit-rot it may not
work out-of-the-box.

### Energy guards for energy-interference-free instrumentation

An energy guard allows the target application to execute (instrumentation)
code of an arbitrary energy cost, while running on an intermittent
power supply. For example, a costly invariant check can be wrapped into
an energy guard:

    ENERGY_GUARD_BEGIN();
    [ code ]
    ENERGY_GUARD_END();

While executing energy guarded code, EDB powers the target. At the end of the
guard, EDB restores the energy level to its value before the energy guard,
masking the effect of the guard on the stored energy level.

Build the app with `TEST_ENERGY_GUARDS = 1` and the rest set to 0.

The test code contains a watchpoint before and after the guard. If all is
working, we should see no difference in the energy values at the watchpoints
(except for some precision error), despite the lengthy code inside the guard.


    > watch --energy 0 E
    > watch --energy 1 E
    > stream watchpoints
    adc_sampling_period_cycles=3000
    host_timestamp_sec,timestamp_sec,watchpoint_id,watchpoint_vcap
    7.999799,0.016739,0,2.3663
    7.999799,0.020505,1,2.3437
    7.999799,0.020784,0,2.3422
    7.999799,0.002843,1,2.3131

### Keep-alive asserts

With keep-alive asserts, an application can check invariants while running on
an intermittent power supply. When an assert fails, EDB steps in, starts
supplying power, and presents the user with an interactive debug prompt.

Build the app with `TEST_ASSERT = 1` and the rest set to 0.

    > wait

Turn on the RF power source. Then, once the ASSERT is reached in code, the
console should indicate which assert failed, and allow further inspection:

    Interrupted: 'ASSERT' line: 57 Vcap_saved = 2.3517
    *> pc
    0x0000473c
    *>

### Interactive debugging

#### Interrupt on next boot

Build the app with `TEST_INTERRUPT = 1` and the rest set to 0.

[Turn off the reader](#sllurp-toolbox-for-rfid-reader) (to make things
simpler), then charge the WISP and interrupt the execution. The `int` command
tells EDB to wait for WISP to boot up, and then to interrupt:

        > int

Turn on the RFID reader. After serveral seconds, depending on the RF power,
once the WISP boots, the console should drop into the interactive debug mode:

        Vcap_saved = 2.3320
        *> pc
        0x000046ee
        *> cont
        Vcap_restored = 2.3284

While in interactive debug mode, the console prompt should be marked with a
`*`, like `*>`. Also, the orange LED on the EDB board should be on and the
green LED on the side of the WISP without the MCU should also be on.

#### Breakpoints

Build the app with `TEST_BREAKPOINTS = 1` and the rest set to 0.

The interactive debug mode can also be reached by hitting a breakpoint. In
the application code, add

    EXTERNAL_BREAKPOINT(1);

Then, [turn on the power source](#sllurp-toolbox-for-rfid-reader), enable the
breakpoint in the EDB console, and wait for it to be hit:

    > break 0 E
    > break 1 E
    > wait
    Interrupted: BREAKPOINT id: 1 Vcap_saved = 2.2336
    *>

*TODO*: Explain the types of breakpoints (`--type`)

In interactive mode, we can get the current program counter address:

    *> pc
    0x00004766

In interactive mode, any address (volatile memory, non-volatile memory,
memory-mapped registers) can be read and written. Currently, EDB cannot resolve
symbols into addresses, so this has to be done manually. For example, get the
address of a global (non-volatile) variable:

    msp430-elf-nm edb-rat.out | grep nv_counter
    0000440c D nv_counter

Note: to get the `msp430-elf-nm` add TI GCC toolchain to your `PATH`: `export
PATH=$PATH:/opt/ti/mspgcc/bin`.

Inspect and modify the value of that global variable in the EDB console (when
in interactive session):

    *> read 0x440c 2
    0000440c: 9c 63
    *> write 0x440c 00 00
    *> read 0x440c 2
    0000440c: 00 00

To continue the execution (and have EDB automatically restore the energy level
its value prior to interruption):

    *> cont
