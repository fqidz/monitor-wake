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

int main(void)
{
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
                    printf("[%li] awake\n", time(NULL));
                }
            }
        }

        // free the message
        dbus_message_unref(msg);
    }
}
