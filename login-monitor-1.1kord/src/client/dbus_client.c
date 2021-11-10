#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dbus/dbus.h>

int main ()
{

    DBusConnection *conn;
	DBusError err;
    DBusMessage* msg;
    DBusMessageIter args;
    uid_t uid;

    int ret=0;

    dbus_error_init(&err);//将错误对象连接到dbus

    /* connect to the daemon bus */
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!conn) {
		fprintf(stderr, "Failed to get a system DBus connection: %s\n", err.message);
		exit(1);
	}

    // add a rule for which messages we want to see
    dbus_bus_add_match(conn,
                       "type='signal',interface='com.kylin.intel.edu.uregis.interface'",
                       &err); // see signals from the given interface

    dbus_bus_add_match(conn,
                    "type='signal',interface='org.freedesktop.login1.Manager'",
                    &err); // see signals from the given interface

    dbus_connection_flush(conn);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        exit(1);
    }

    // loop listening for signals being emmitted
    while (true) {
     
        // non blocking read of the next available message
        dbus_connection_read_write(conn, 0);
        msg = dbus_connection_pop_message(conn);
     
        // loop again if we haven't read a message
        if (NULL == msg) {
            sleep(1);
            continue;
        }
     
        // check if the message is a signal from the correct interface and with the correct name
        if (dbus_message_is_signal(msg, "com.kylin.intel.edu.uregis.interface", "UserLogin")) {
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n");
            else if (DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&args))
                fprintf(stderr, "Argument is not uint!\n");
            else {
                dbus_message_iter_get_basic(&args, &uid);
                printf("Got UserLogin Signal with uid value %d\n", uid);
            }
        }

        if (dbus_message_is_signal(msg, "com.kylin.intel.edu.uregis.interface", "UserLogout")) {
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n");
            else if (DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&args))
                fprintf(stderr, "Argument is not uint!\n");
            else {
                dbus_message_iter_get_basic(&args, &uid);
                printf("Got UserLogout Signal with uid value %d\n", uid);
            }
        }

        if (dbus_message_is_signal(msg, "org.freedesktop.login1.Manager", "UserNew")) {
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n");
            else if (DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&args))
                fprintf(stderr, "Argument is not uint!\n");
            else {
                dbus_message_iter_get_basic(&args, &uid);
                printf("Got UserNew Signal with uid value %d\n", uid);
            }
        }

        if (dbus_message_is_signal(msg, "org.freedesktop.login1.Manager", "UserRemoved")) {
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n");
            else if (DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&args))
                fprintf(stderr, "Argument is not uint!\n");
            else {
                dbus_message_iter_get_basic(&args, &uid);
                printf("Got UserRemoved Signal with uid value %d\n", uid);
            }
        }
     
        // free the message
        dbus_message_unref(msg);
    }
    return ret;
}