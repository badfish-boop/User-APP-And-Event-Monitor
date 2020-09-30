#ifndef _DBUS_SERVER_H
#define _DBUS_SERVER_H

#include <string.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> /* for glib main loop */
#include "login_monitor.h"

GMainLoop *mainloop;

struct dbus_set_para
{
    char *server_name;
    char *object_path;
    char *interface;
    char *method;
};

struct dbus_Get_method_para
{
    char *dbus_get_interface ;
    char *dbus_get_property ;
};

DBusHandlerResult server_message_handler(DBusConnection *conn, DBusMessage *message, void *data);

void dbus_server_loop(DBusConnection *conn);

void dbus_userlogin_singal_send(uid_t ruid);

void dbus_userlogout_singal_send(uid_t ruid);

void dbus_pkgadd_singal_send(char* pkgname);

void dbus_pkgremove_singal_send(char* pkgname);

void dbus_systime_change_singal_send(char *front_time,char *current_time);

int dbus_list_users_method_call(struct dbus_list_users list_user[]);

char * dbus_get_method_call(struct dbus_set_para dbus_para);

uid_t dbus_get_login_self_uid(struct dbus_set_para dbus_para);

#endif