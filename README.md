edb-rat: an app for testing EDB features
========================================

This quick start describes how to use EDB to debug an application.

### Prep

Clone this repository: it contains a "lab rat" test application on which we
will demonstrate features of EDB. This repository also includes as submodules
the EDB firmware and host console as an external tool dependency (in `ext/`) --
this is done to make the procedure as reproducible as possible, submodules
freeze versions of interdependent components.

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

Open EDB console:

    cd ext/edb-console
    ./edb

### EDB commands

Connect workstation to the EDB board:

    > attach

Sense voltage on the target capacitor

    > sense vcap
