#include "dbus/dbus-shared.h"
#include <dbus-1.0/dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

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

    // Request a name on the bus.
    // monitor-wake.conf needs to be in /usr/share/dbus-1/system.d/ to allow user to
    // own the bus name.
    ret = dbus_bus_request_name(conn, "user.MonitorWake",
                                DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s: %s)\n", err.name, err.message);
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
        if (dbus_message_is_signal(msg, "org.freedesktop.login1.Manager", "PrepareForSleep")) {
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n");
            else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
                fprintf(stderr, "Argument is not string!\n");
            else {
                dbus_message_iter_get_basic(&args, &sigvalue);
                printf("Got Signal with value %i\n", sigvalue);
            }
        }

        // free the message
        dbus_message_unref(msg);
    }

    // if (conn) {
    //     dbus_connection_close(conn);
    //     dbus_connection_unref(conn);
    // }
    //
    // return 0;
}
