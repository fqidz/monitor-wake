// Mostly based off of Matthew Johnson's dbus tutorials:
// https://www.matthew.ath.cx/misc/dbus
// https://www.matthew.ath.cx/projects.git/dbus-example.c

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

/* Connect to dbus system bus. Although we don't need any permissions to
   connect to the system bus, we do need permissions to request a bus name. */
DBusConnection *connect_system_bus(DBusError *error)
{
    DBusConnection *connection = dbus_bus_get(DBUS_BUS_SYSTEM, error);
    if (dbus_error_is_set(error)) {
        fprintf(stderr, "Connection Error (%s)\n", error->message);
        dbus_error_free(error);
    }
    if (NULL == connection) {
        exit(1);
    }
    return connection;
}

/* Main loop where we open a dbus connection, request a name on the bus, and
   listen & output text for when 'org.freedesktop.login1.Manager' sends a `false`
   'PrepareSleep' signal. */
__attribute__((noreturn)) void main_dbus_loop(int arg_timestamp)
{
    assert(arg_timestamp >= 0 && arg_timestamp <= 2);

    DBusError err;
    // initialise the errors
    dbus_error_init(&err);

    DBusConnection *conn = connect_system_bus(&err);
    // We don't need to request a bus name as we're only reading signals
    // and don't really require a "named" connection

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
        // Non-blocking read of the next available message.
        // TODO: async reading so we don't have to poll
        dbus_connection_read_write(conn, 0);
        msg = dbus_connection_pop_message(conn);

        // loop again if we haven't read a message
        if (NULL == msg) {
            sleep(1);
            continue;
        }

        int sigvalue;

        // check if the message is a signal from the correct interface and with the correct name
        if (dbus_message_is_signal(msg, "org.freedesktop.login1.Manager",
                                   "PrepareForSleep")) {
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n");
            else if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&args))
                fprintf(stderr, "Argument is not bool!\n");
            else {
                // 'PrepareForSleep' is true when device prepares for sleep,
                // and false when it wakes up.
                //
                // No purpose in outputting text when the device prepares for
                // sleep because you'd need an inhibit lock to block sleeping
                // to actually execute something.
                //
                // More info here:
                // https://www.freedesktop.org/wiki/Software/systemd/inhibit/
                // before the device sleeps.

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
        main_dbus_loop(ARG_NO_TIMESTAMP); // never returns
    }

    if (argc > 2) {
        fprintf(stderr,
                "monitor-wake: expected only one argument. See `monitor-wake --help`\n");
        return 1;
    }

    char *arg = argv[1];

    if (strcmp(arg, "-u") == 0 || strcmp(arg, "--unix-timestamp") == 0) {
        main_dbus_loop(ARG_UNIX_TIMESTAMP); // never returns
    } else if (strcmp(arg, "-t") == 0 || strcmp(arg, "--timestamp") == 0) {
        main_dbus_loop(ARG_TIMESTAMP); // never returns
    } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
        printf(VERSION_TEXT);
        return 0;
    } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
        printf(HELP_TEXT);
        return 0;
    }
}
