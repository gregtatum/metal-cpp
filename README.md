# Install dependencies

Run `make init`

## Basic Usage

There is a small live-reload and build script available under `make watch`. There are
a variety of shortcuts available from this script, which is all powered by the makefile.
For instance, you can clean the object files, restart the example, kill the process,
quit, and do a GPU trace.

`make watch EXAMPLE=bunny`

## Building

The examples are built using the name of the cpp file, and the `bin` directory. By default
the examples are built in debug mode, with safer instrumentation and assertion checks.

`make ./bin/bunny`

To build for release, add `RELEASE=1` to the example.

`make ./bin/bunny RELEASE=1`

To run the example:

`./bin/bunny`

## Environment variables

`LOG_SHADER_CALLS=1 ./bin/bunny` - Logs the first shader call.
