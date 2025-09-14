// https://www.matthew.ath.cx/misc/dbus
// https://www.matthew.ath.cx/projects.git/dbus-example.c

#include "dbus/dbus-protocol.h"
#include "dbus/dbus-shared.h"
#include <dbus-1.0/dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#define ARG_NO_TIMESTAMP 0
#define ARG_UNIX_TIMESTAMP 1
#define ARG_TIMESTAMP 2

__attribute__((noreturn)) void main_dbus_loop(int arg_timestamp)
{
    assert(arg_timestamp >= 0 && arg_timestamp <= 2);
    DBusError err;
    DBusConnection *conn;
    int ret;
    // initialise the errors
    dbus_error_init(&err);

    // connect to the bus
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        exit(1);
    }

    // request a name on the bus
    ret = dbus_bus_request_name(conn, "user.MonitorWake",
                                DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        if (strcmp(DBUS_ERROR_ACCESS_DENIED, err.name) == 0) {
            fprintf(stderr,
                    "%s: %s (Check the man page or visit https://github.com/fqidz/monitor-wake for ways to allow access)\n",
                    err.name, err.message);
        } else {
            fprintf(stderr, "Name Error (%s: %s)\n", err.name, err.message);
        }
        dbus_error_free(&err);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
        exit(1);
    }

    // add a rule for which messages we want to see
    dbus_bus_add_match(
            conn,
            "type='signal',interface='org.freedesktop.login1.Manager',member='PrepareForSleep'",
            &err); // see signals from the given interface
    dbus_connection_flush(conn);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        exit(1);
    }

    DBusMessage *msg;
    DBusMessageIter args;

    // loop listening for signals being emitted
    while (true) {
        // non blocking read of the next available message
        dbus_connection_read_write(conn, 0);
        msg = dbus_connection_pop_message(conn);

        int sigvalue;

        // loop again if we haven't read a message
        if (NULL == msg) {
            sleep(1);
            continue;
        }

        // check if the message is a signal from the correct interface and with the correct name
        if (dbus_message_is_signal(msg, "org.freedesktop.login1.Manager",
                                   "PrepareForSleep")) {
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n");
            else if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&args))
                fprintf(stderr, "Argument is not bool!\n");
            else {
                dbus_message_iter_get_basic(&args, &sigvalue);
                if (sigvalue == 0) {
                    switch (arg_timestamp) {
                    case ARG_NO_TIMESTAMP: {
                        printf("woken\n");
                        break;
                    }
                    case ARG_UNIX_TIMESTAMP: {
                        printf("%li\n", time(NULL));
                        break;
                    }
                    case ARG_TIMESTAMP: {
                        time_t unix_time;
                        time(&unix_time);

                        printf("%s", ctime(&unix_time));
                        break;
                    }
                    default:
                        __builtin_unreachable();
                    }
                }
            }
        }

        // free the message
        dbus_message_unref(msg);
    }
}

#define HELP_TEXT \
    "Usage: monitor-wake [ -h | --help | -u | --unix-timestamp | -t | --timestamp | --version ]\n\
Monitor and output text when device wakes up from sleep.\n\
\n\
    -u, --unix-timestamp    Outputs unix timestamp instead of just 'woken'\n\
    -t, --timestamp         Outputs human readable timestamp instead of just 'woken'\n\
    -v, --version           Display version and exit\n\
    -h, --help              Display this help text and exit\n"

#define VERSION_TEXT "monitor-wake 1.0.0\n"

int main(int argc, char **argv)
{
    if (argc == 1) {
        main_dbus_loop(ARG_NO_TIMESTAMP);
        // never returns
    }

    if (argc > 2) {
        fprintf(stderr,
                "monitor-wake: expected only one argument. See `monitor-wake --help`\n");
        return 1;
    }

    char *arg = argv[1];

    if (strcmp(arg, "-u") == 0 || strcmp(arg, "--unix-timestamp") == 0) {
        main_dbus_loop(ARG_UNIX_TIMESTAMP);
        // never returns
    } else if (strcmp(arg, "-t") == 0 || strcmp(arg, "--timestamp") == 0) {
        main_dbus_loop(ARG_TIMESTAMP);
        // never returns
    } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
        printf(VERSION_TEXT);
        return 0;
    } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
        printf(HELP_TEXT);
        return 0;
    }
}
