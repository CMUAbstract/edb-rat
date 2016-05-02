edb-rat: an app for testing EDB features
========================================

This repository contains a "lab rat" test application on which we
will demonstrate features of EDB. 

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

Build and flash EDB firmware:

    cd ext/edb-firmware
    make bld/gcc/all
    make bld/gcc/prog
    cd ..

Build and flash edb-rat app:

    make bld/gcc/all
    make bld/gcc/prog

Connect the EDB to the WISP by connecting the two ends of the matching
8+3-pin board-to-board headers. Then, separately, connect three additional
wires: the Vcap line, RF RX line and RF TX line.

Open the EDB console and attach to the debugger

    cd ext/edb-console
    ./edb
    > attach
    >

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

## EDB commands

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

    > stream - - vcap
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

The application code contains watchpoints at some locations of interest,
inserted using the `WATCHPOINT()` macro (TODO: link into app code):

   WATCHPOINT(1);
   while (1) {
     WATCHPOINT(2);
     ...
   }

In the EDB console, enable the watchpoints and start monitoring for them using
the `stream` command:

    > attach
    > watch 1 en vcap
    > watch 2 en vcap
    > stream - - watchpoints
    adc_sampling_period_cycles= 3000
    timestamp_sec,watchpoint_id,watchpoint_vcap
    0.035750,1,2.3291
    0.036672,2,2.2854
    0.037608,2,2.2388
    0.038543,2,2.2147
    0.039479,2,2.2038
    0.040414,2,2.1863
    0.041350,2,2.1455
    0.042286,2,2.1214

The `watch` command supports an optional third argument `vcap`/`novcap` which
controls whether a snapshot of capacitor voltage level is made (the third
column in the above output).

#### Stream printf output

The application code contains printf or log statements with values
of interest (TODO: link into app code):

    while (1) {
     x++;
     PRINTF("x=%x\n", x);
   }

In the EDB console, expect the stdout data using the `wait` command:

    > attach
    > wait
    x=1
    x=2

'''NOTE''': This has not been tested in a while, so due to bit-rot it may not
work out-of-the-box.

#### Stream RFID messages

For applications that communicate with the reader over RFID protocol, EDB can
decode the incoming and (partially) outgoing messages (TODO: add to app).

    > attach
    > stream - - rf_events
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

'''NOTE''': This has not been tested in a while, so due to bit-rot it may not
work out-of-the-box.

### Interactive debugging

[Turn off the reader](#sllurp-toolbox-for-rfid-reader) (to make things
simpler), then charge the WISP and interrupt the execution. The `int` command
tells EDB to wait for WISP to boot up (by watching the regulated voltage,
Vreg), and then to interrupt:

    > charge 2.4; int
    Vcap = 2.4005
    Vcap_saved = 2.2825
    *>

The green LED on the EDB board should be on and the green LED on the side of
the WISP without the MCU should also be on, indicating interactive debug mode.
This is also indicated by the `*` in the EDB console prompt.

The interactive debug mode can also be reached by hitting a breakpoint. In
the application code, add

    BREAKPOINT(1);

Then, [turn on the power source](#sllurp-toolbox-for-rfid-reader), enable the
breakpoint in the EDB console, and wait for it to be hit:

    > break e 1 en
    > wait
    Interrupted: BREAKPOINT id: 1 Vcap_saved = 2.2336
    *>

''Sidenote'': The first argument to the `break` cmd 'e' stands for 'external',
the type of breakpoint. TODO: elaborate.

In interactive mode, any address (volatile memory, non-volatile memory,
memory-mapped registers) can be read and written. Currently, EDB cannot resolve
symbols into addresses, so this has to be done manually. For example, get the
address of a global variable from the `.map` file, generated by the linker:

    grep nv_state_magic bld/gcc/*.map
    00004400  nv_state_magic
    00004400  nv_state_magic

Inspect and modify the value of that global variable in the EDB console (when
in interactive session):

    *> read 0x4400 4
    0x00004400: 0xef 0xbe 0xad 0xde
    *> write 0x4400 0xfe 0xca 0x0d 0xf0
    0x00004400: 0xfe 0xca 0x0d 0xf0

To continue the execution (and have EDB automatically restore the energy level
its value prior to interruption):

    *> cont

### Intermittence-aware debugging primitives

TODO: introduce _energy guards_ and _keep-alive asserts_.
