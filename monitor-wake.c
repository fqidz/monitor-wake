#include <stdio.h>
#include <dbus-1.0/dbus/dbus.h>

// DBusError err;
// DBusConnection* conn;
// int ret;
// // initialise the errors
// dbus_error_init(&err);
//
int main(void) {
    printf("hello\n");
   // // connect to the bus
   // conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
   // if (dbus_error_is_set(&err)) {
   //    fprintf(stderr, "Connection Error (%s)\n", err.message);
   //    dbus_error_free(&err);
   // }
   // if (NULL == conn) {
   //    exit(1);
   // }
   //
   // // request a name on the bus
   // ret = dbus_bus_request_name(conn, "test.method.server",
   //       DBUS_NAME_FLAG_REPLACE_EXISTING
   //       , &err);
   // if (dbus_error_is_set(&err)) {
   //    fprintf(stderr, "Name Error (%s)\n", err.message);
   //    dbus_error_free(&err);
   // }
   // if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
   //    exit(1);
   // }

    return 0;
}
