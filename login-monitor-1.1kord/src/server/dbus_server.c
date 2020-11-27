#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <error.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> /* for glib main loop */
#include "dbus_server.h"
#include "login_monitor.h"

DBusConnection *dbus_conn;

const char *server_introspection_xml =
	DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
	"<node>\n"

    "  <interface name='org.freedesktop.DBus.Introspectable'>\n"
	"    <method name='Introspect'>\n"
	"       <arg name='data' type='s' direction='out' />\n"
	"    </method>\n"
    "  </interface>\n"

	"  <interface name='com.kylin.intel.edu.uregis.interface'>\n"
    "    <method name='GetLoginList'>\n"
	"      <arg name='uid_list' type='s' direction='out' />\n"
	"    </method>\n"
    "    <method name='GetUserDetails'>\n"
	"       <arg name='uid' type='u' direction='in' />\n"
    "       <arg name='username' type='s' direction='out' />\n"
    "       <arg name='tencent_uid' type='s' direction='out' />\n"
    "       <arg name='group' type='u' direction='out' />\n"
    "       <arg name='home_dir' type='s' direction='out' />\n"
	"    </method>\n"
    "    <method name='GetCurUser'>\n"
	"      <arg name='uid' type='u' direction='out' />\n"
	"    </method>\n"
    "    <signal name='PkgAdd'>\n"
    "        <arg name='appid' type='s' direction='out' />\n"
	"    </signal>"
    "    <signal name='PKgRemove'>\n"
    "       <arg name='appid' type='s' direction='out' />\n"
	"    </signal>"
	"    <signal name='UserLogin'>\n"
    "       <arg name='uid' type='u' direction='out' />\n"
	"    </signal>"
    "    <signal name='UserLogout'>\n"
    "       <arg name='uid' type='u' direction='out' />\n"
	"    </signal>"
    "    <signal name='Timechange'>\n"
    "       <arg name='front_time' type='s' direction='out' />\n"
    "       <arg name='current_time' type='s' direction='out' />\n"
	"    </signal>"
	"  </interface>\n"

"</node>\n";

DBusConnection *Reg_New_SystemBus()
{
    DBusConnection *connection;
    DBusError error;

    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

    if(!connection)
    {
        printf("无法新建dbus连接，%s\n",error.message);
        return NULL;
    }

    return connection;
}

int dbus_list_users_method_call(struct dbus_list_users list_user[])
{
    printf("enter dbus_list_users_method_call \n");
    DBusMessage* msg;
    DBusMessageIter args,subargs,ssubargs;
    DBusPendingCall* pending;
    DBusConnection *conn;
    int users_num=0;

    conn=Reg_New_SystemBus();
    
    msg = dbus_message_new_method_call("org.freedesktop.login1", // target for the method call
                                       "/org/freedesktop/login1", // object to call on
                                       "org.freedesktop.login1.Manager", // interface to call on
                                       "ListUsers"); // method name
    if (NULL == msg) {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }
/*     
    // append arguments
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &param)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
*/     
    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) { // -1 is default timeout
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    if (NULL == pending) {
        fprintf(stderr, "Pending Call Null\n");
        exit(1);
    }
    dbus_connection_flush(conn);
     
    // free message
    dbus_message_unref(msg);


     
    // block until we receive a reply
    dbus_pending_call_block(pending);
     
    // get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg) {
        fprintf(stderr, "Reply Null\n");
        exit(1);
    }
    // free the pending message handle
    dbus_pending_call_unref(pending);

    printf("enter dbus_list_users_method_call replay args\n");
    // read the parameters
    if (!dbus_message_iter_init(msg, &args))
        fprintf(stderr, "Message has no arguments!\n");
    while (DBUS_TYPE_INVALID != dbus_message_iter_get_arg_type(&args)) {

        switch (dbus_message_iter_get_arg_type(&args)) {
			case DBUS_TYPE_STRING:
				break;
			case DBUS_TYPE_INT32:
				break;
			case DBUS_TYPE_ARRAY:
				dbus_message_iter_recurse(&args, &subargs);
				while (dbus_message_iter_get_arg_type(&subargs)
						!= DBUS_TYPE_INVALID) 
				{
                    
					if(DBUS_TYPE_STRUCT == dbus_message_iter_get_arg_type(&subargs)) 
					{
                    
                        dbus_message_iter_recurse(&subargs, &ssubargs);
                        while (dbus_message_iter_get_arg_type(&ssubargs)
                                != DBUS_TYPE_INVALID) 
                        {
                            
                            switch (dbus_message_iter_get_arg_type(&ssubargs)) 
                            {
                                char *s;
                                case DBUS_TYPE_OBJECT_PATH:
                                    dbus_message_iter_get_basic(&ssubargs, &s);
                                    strcpy(list_user[users_num].object_path,s);
                                    break;
                                case DBUS_TYPE_STRING:
                                    dbus_message_iter_get_basic(&ssubargs, &s);
                                    strcpy(list_user[users_num].user_name,s);
                                    break;
                                case DBUS_TYPE_UINT32:
                                    dbus_message_iter_get_basic(&ssubargs, &list_user[users_num].uid);
                                    printf("uid=%d,i=%d\n",list_user[users_num].uid,users_num);
                                    break;
                            }
                            dbus_message_iter_next(&ssubargs);
                        }
					}
					dbus_message_iter_next(&subargs);
                    printf("object_path=%s\n",list_user[users_num].object_path);
                    users_num++;                       //每登录一个用户则+1
				}
                break;
		}
		dbus_message_iter_next(&args);
    }
     
    // free reply and close connection
    dbus_message_unref(msg);
    return users_num;
}

uid_t dbus_get_login_self_uid(struct dbus_set_para dbus_para)
{
    printf("enter dbus_get_login_self_uid \n");
    DBusMessage* msg;
    DBusMessageIter args,subargs;
    DBusPendingCall* pending;
    DBusConnection *conn;
    struct dbus_Get_method_para Get_method_para;
    Get_method_para.dbus_get_interface = "org.freedesktop.login1.User";
    Get_method_para.dbus_get_property = "UID";
    uid_t uid = 0;

    conn=Reg_New_SystemBus();
     
    msg = dbus_message_new_method_call(dbus_para.server_name, // target for the method call
                                       dbus_para.object_path, // object to call on
                                       dbus_para.interface, // interface to call on
                                       dbus_para.method); // method name
    if (NULL == msg) {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }

    // append arguments
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &Get_method_para.dbus_get_interface)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }

    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &Get_method_para.dbus_get_property)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    
    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) { // -1 is default timeout
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    if (NULL == pending) {
        fprintf(stderr, "Pending Call Null\n");
        exit(1);
    }
    dbus_connection_flush(conn);
     
    // free message
    dbus_message_unref(msg);


    // block until we receive a reply
    dbus_pending_call_block(pending);
     
    // get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg) {
        fprintf(stderr, "Reply Null\n");
        exit(1);
    }
    // free the pending message handle
    dbus_pending_call_unref(pending);

    printf("enter dbus_list_users_method_call replay args\n");
    // read the parameters
    if (!dbus_message_iter_init(msg, &args))
        fprintf(stderr, "Message has no arguments!\n");
    
    if (DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&args))
    {
        dbus_message_iter_recurse(&args, &subargs);
        if (DBUS_TYPE_UINT32 == dbus_message_iter_get_arg_type(&subargs))
        {
            dbus_message_iter_get_basic(&subargs, &uid);
            printf("Get Method called obtain uid is %d\n", uid);
        }
    }
    return uid;
}

char *dbus_get_method_call(struct dbus_set_para dbus_para)
{
    printf("enter dbus_get_method_call \n");
    DBusMessage* msg;
    DBusMessageIter args,subargs;
    DBusPendingCall* pending;
    DBusConnection *conn;
    char *dbus_get_interface = "org.freedesktop.login1.User";
    char *dbus_get_property = "State";
    char *state = "";

    conn=Reg_New_SystemBus();
     
    msg = dbus_message_new_method_call(dbus_para.server_name, // target for the method call
                                       dbus_para.object_path, // object to call on
                                       dbus_para.interface, // interface to call on
                                       dbus_para.method); // method name
    if (NULL == msg) {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }

    // append arguments
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &dbus_get_interface)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }

    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &dbus_get_property)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    
    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) { // -1 is default timeout
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    if (NULL == pending) {
        fprintf(stderr, "Pending Call Null\n");
        exit(1);
    }
    dbus_connection_flush(conn);
     
    // free message
    dbus_message_unref(msg);


    // block until we receive a reply
    dbus_pending_call_block(pending);
     
    // get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg) {
        fprintf(stderr, "Reply Null\n");
        exit(1);
    }
    // free the pending message handle
    dbus_pending_call_unref(pending);

    printf("enter dbus_list_users_method_call replay args\n");
    // read the parameters
    if (!dbus_message_iter_init(msg, &args))
        fprintf(stderr, "Message has no arguments!\n");
    
    if (DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&args))
    {
        dbus_message_iter_recurse(&args, &subargs);
        if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&subargs))
        {
            dbus_message_iter_get_basic(&subargs, &state);
            printf("Get Method called obtain state is %s\n", state);
        }
    }
    return state;
}

char *dbus_get_tencent_id_method_call(struct dbus_set_para dbus_para, char *username)
{
    printf("enter dbus_get_tencent_id_method_call \n");
    DBusMessage* msg;
    DBusMessageIter args;
    DBusPendingCall* pending;
    DBusConnection *conn;

    char *tencent_id = "";

    conn=Reg_New_SystemBus();
     
    msg = dbus_message_new_method_call(dbus_para.server_name, // target for the method call
                                       dbus_para.object_path, // object to call on
                                       dbus_para.interface, // interface to call on
                                       dbus_para.method); // method name
    if (NULL == msg) {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }

    // append arguments
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &username)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    
    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) { // -1 is default timeout
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    if (NULL == pending) {
        fprintf(stderr, "Pending Call Null\n");
        exit(1);
    }
    dbus_connection_flush(conn);
     
    // free message
    dbus_message_unref(msg);


    // block until we receive a reply
    dbus_pending_call_block(pending);
     
    // get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg) {
        fprintf(stderr, "Reply Null\n");
        exit(1);
    }
    // free the pending message handle
    dbus_pending_call_unref(pending);

    printf("enter dbus_list_users_method_call replay args\n");
    // read the parameters
    if (!dbus_message_iter_init(msg, &args))
        fprintf(stderr, "Message has no arguments!\n");
    
    if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args))
    {
        dbus_message_iter_get_basic(&args, &tencent_id);
        printf("Get Method called obtain tencent_id is %s\n", tencent_id);
    }
    
    return tencent_id;
}

void dbus_userlogin_singal_send(uid_t ruid)
{
    printf("enter dbus_userlogin_singal_send...\n");

    DBusMessage* msg;
    DBusMessageIter args;
     
    // create a signal and check for errors
    msg = dbus_message_new_signal("/com/kylin/intel/edu/uregis", // object name of the signal
                                  "com.kylin.intel.edu.uregis.interface", // interface name of the signal
                                  "UserLogin"); // name of the signal
    if (NULL == msg)
    {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }
     
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &ruid)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
     
    // send the message and flush the connection
    if (!dbus_connection_send(dbus_conn, msg, NULL)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    dbus_connection_flush(dbus_conn);
     
    // free the message
    dbus_message_unref(msg);


}

void dbus_userlogout_singal_send(uid_t ruid)
{
    printf("enter dbus_userlogout_singal_send...\n");

    DBusMessage* msg;
    DBusMessageIter args;
     
    // create a signal and check for errors
    msg = dbus_message_new_signal("/com/kylin/intel/edu/uregis", // object name of the signal
                                  "com.kylin.intel.edu.uregis.interface", // interface name of the signal
                                  "UserLogout"); // name of the signal
    if (NULL == msg)
    {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }
     
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &ruid)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
     
    // send the message and flush the connection
    if (!dbus_connection_send(dbus_conn, msg, NULL)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    dbus_connection_flush(dbus_conn);
     
    // free the message
    dbus_message_unref(msg);
    
}

void dbus_pkgadd_singal_send(char* pkgname)
{
    printf("enter dbus_pkgadd_singal_send...\n");

    DBusMessage* msg;
    DBusMessageIter args;
     
    // create a signal and check for errors
    msg = dbus_message_new_signal("/com/kylin/intel/edu/uregis", // object name of the signal
                                  "com.kylin.intel.edu.uregis.interface", // interface name of the signal
                                  "PkgAdd"); // name of the signal
    if (NULL == msg)
    {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }
     
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &pkgname)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
     
    // send the message and flush the connection
    if (!dbus_connection_send(dbus_conn, msg, NULL)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    dbus_connection_flush(dbus_conn);
     
    // free the message
    dbus_message_unref(msg);
    
}

void dbus_pkgremove_singal_send(char* pkgname)
{
    printf("enter dbus_pkgremove_singal_send...\n");

    DBusMessage* msg;
    DBusMessageIter args;
     
    // create a signal and check for errors
    msg = dbus_message_new_signal("/com/kylin/intel/edu/uregis", // object name of the signal
                                  "com.kylin.intel.edu.uregis.interface", // interface name of the signal
                                  "PKgRemove"); // name of the signal
    if (NULL == msg)
    {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }
     
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &pkgname)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
     
    // send the message and flush the connection
    if (!dbus_connection_send(dbus_conn, msg, NULL)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    dbus_connection_flush(dbus_conn);
     
    // free the message
    dbus_message_unref(msg);
    
}

void dbus_systime_change_singal_send(char *front_time,char *current_time)
{
    printf("enter dbus_systime_change_singal_send...\n");

    DBusMessage* msg;
    DBusMessageIter args;
     
    // create a signal and check for errors
    msg = dbus_message_new_signal("/com/kylin/intel/edu/uregis", // object name of the signal
                                  "com.kylin.intel.edu.uregis.interface", // interface name of the signal
                                  "Timechange"); // name of the signal
    if (NULL == msg)
    {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }
     
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &front_time)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }

    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &current_time)) {
    fprintf(stderr, "Out Of Memory!\n");
    exit(1);
    }
     
    // send the message and flush the connection
    if (!dbus_connection_send(dbus_conn, msg, NULL)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    dbus_connection_flush(dbus_conn);
     
    // free the message
    dbus_message_unref(msg);
    
}

DBusHandlerResult server_message_handler(DBusConnection *conn, DBusMessage *message, void *data)
{
	DBusHandlerResult result=0;
    DBusMessage *reply = NULL;
    DBusMessageIter args;
	DBusError err;
    bool quit = false;

	fprintf(stderr, "Got D-Bus request: %s.%s on %s\n",
		dbus_message_get_interface(message),
		dbus_message_get_member(message),
		dbus_message_get_path(message));
    dbus_error_init(&err);

    if (!(reply = dbus_message_new_method_return(message))) //申请内存适配则事先退出
    {
        result = DBUS_HANDLER_RESULT_NEED_MEMORY;
        return result;
    }

	if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {

		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &server_introspection_xml,
					 DBUS_TYPE_INVALID);

    }else if(dbus_message_is_method_call(message, "com.kylin.intel.edu.uregis.interface", "Quit")){
         
         g_main_loop_quit(mainloop);     //用于退出主循环 

    }else if (dbus_message_is_method_call(message, "com.kylin.intel.edu.uregis.interface", "GetLoginList")) {

		char* param = "";
        char **login_user_name;
        login_user_name=(char **)malloc(1024*sizeof(char *));
        int user_num=0;
        int uid_list[10]={0};
        struct passwd *pwd;
        int error_code = -2;
        int error_code_one = -3;
 
        // read the arguments
        if (!dbus_message_iter_init(message, &args))
            fprintf(stderr, "Message has no arguments!\n");
        else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
            fprintf(stderr, "Argument is not string!\n");
        else
            dbus_message_iter_get_basic(&args, &param);
        printf("Method called with %s\n", param);
        
        if (!(reply = dbus_message_new_method_return(message)))
			goto fail;
        
        if(get_user_info(login_user_name,&user_num))
        {
            printf(" get_user_info error,now no uesr login! \n");
            dbus_message_iter_init_append(reply, &args);
            if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &error_code_one)) {
                    fprintf(stderr, "Out Of Memory!\n");
                    exit(1);
            }
            goto send;
        }
        printf("user_num=%d\n",user_num);
        for(int i=0;i<user_num;i++)
        {
            printf("login_user_name = %s\n",login_user_name[i]);
            pwd=getpwnam(login_user_name[i]);        //using getpwnam funtion obtian psaawd struct by user name
            if(pwd==NULL)
            {
                printf(" getpwnam exe error \n");
                dbus_message_iter_init_append(reply, &args);
                if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &error_code)) {
                    fprintf(stderr, "Out Of Memory!\n");
                    exit(1);
                }
                goto send;
            }
            uid_list[i]=pwd->pw_uid;
            printf("login_user_name=%s,uid_list=%d\n",login_user_name[i],uid_list[i]);
        }

        free(login_user_name);

		 // add the arguments to the reply
        dbus_message_iter_init_append(reply, &args);
        for(int i=0;i<user_num;i++)
        {
            int *tmp = uid_list[i];
            if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &tmp)) {
                fprintf(stderr, "Out Of Memory!\n");
                exit(1);
            }
        }
        goto send;
    
    }else if (dbus_message_is_method_call(message, "com.kylin.intel.edu.uregis.interface", "GetUserDetails")) {

		uid_t uid = 0;
        struct passwd *pwd = NULL;
        char *username = "";
        char * tencent_uid ="this user don't have tencent uid";
        gid_t group = 0;
        char *home_dir = "";
        const char *msg;
        int error_code_1 = -1;
        int error_code_2 = -2;

        if(!dbus_message_get_args(message,&err,DBUS_TYPE_UINT32,&msg,DBUS_TYPE_INVALID))  //判断传入参数是否合法
        {
            goto fail;
        }
 
        // read the arguments
        if (!dbus_message_iter_init(message, &args))
            fprintf(stderr, "Message has no arguments!\n");
        else if (DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&args))
            fprintf(stderr, "Argument is not uint!\n");
        else
            dbus_message_iter_get_basic(&args, &uid);
        printf("Method called with %d\n", uid);

        
        if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

        pwd = getpwuid (uid);
        if (pwd == NULL)
        {
            printf("cannot find tencent_uid for user ID %d\n",uid);
            dbus_message_iter_init_append(reply, &args);
            if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &error_code_1)) {
                fprintf(stderr, "Out Of Memory!\n");
                exit(1);
            }
            goto send;
        }

        username = pwd->pw_name;
        tencent_uid=get_tencent_id(username);
        if(!tencent_uid)
        {
            printf("cannot find tencent_uid for user ID %d\n",uid);
            dbus_message_iter_init_append(reply, &args);
            if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &error_code_2)) {
                fprintf(stderr, "Out Of Memory!\n");
                exit(1);
            }
            goto send;
        }
        group = pwd->pw_gid;
        home_dir = pwd->pw_dir;

		 // add the arguments to the reply
        dbus_message_iter_init_append(reply, &args);
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &username)) {
            fprintf(stderr, "Out Of Memory!\n");
            exit(1);
        }
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &tencent_uid)) {
            fprintf(stderr, "Out Of Memory!\n");
            exit(1);
        }
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &group)) {
            fprintf(stderr, "Out Of Memory!\n");
            exit(1);
        }
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &home_dir)) {
            fprintf(stderr, "Out Of Memory!\n");
            exit(1);
        }
        goto send;
        
    }else if (dbus_message_is_method_call(message, "com.kylin.intel.edu.uregis.interface", "GetCurUser")) {
        
        uid_t uid = 0;
        char* param = "";

         // read the arguments
        if (!dbus_message_iter_init(message, &args))
            fprintf(stderr, "Message has no arguments!\n");
        else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
            fprintf(stderr, "Argument is not string!\n");
        else
            dbus_message_iter_get_basic(&args, &param);
        printf("Method called with %s\n", param);
        
        if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

        uid=get_cur_uid();             //通过system dbus 中 org.freedesktop.login1 获取
        //uid=get_current_uid();       //计划通过system dbus 中 org.freedesktop.login1.user.self 获取，失败并弃用

        dbus_message_iter_init_append(reply, &args);
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &uid)) {
            fprintf(stderr, "Out Of Memory!\n");
            exit(1);
        }

        goto send;

    }else
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;


fail:
	if (dbus_error_is_set(&err)) {
		if (reply)
			dbus_message_unref(reply);
		reply = dbus_message_new_error(message, err.name, err.message);
		dbus_error_free(&err);
	}

	/*
	 * In any cases we should have allocated a reply otherwise it
	 * means that we failed to allocate one.
	 */
	if (!reply)
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	/* Send the reply which might be an error one too. */
	result = DBUS_HANDLER_RESULT_HANDLED;
	if (!dbus_connection_send(conn, reply, NULL))
		result = DBUS_HANDLER_RESULT_NEED_MEMORY;
	    dbus_message_unref(reply);

	if (quit) {
		fprintf(stderr, "Server exiting...\n");
		g_main_loop_quit(mainloop);
	}
    return result;
send:
     // send the reply && flush the connection
        if (!dbus_connection_send(conn, reply, NULL)) {
            fprintf(stderr, "Out Of Memory!\n");
            exit(1);
        }
        dbus_connection_flush(conn);
    
        // free the reply
        dbus_message_unref(reply);

    return result;
}

void dbus_server_loop(DBusConnection *conn)
{
    printf("DBus server start success, enter DBus message monitor loop...\n");
    /*
	 * For the sake of simplicity we're using glib event loop to
	 * handle DBus messages. This is the only place where glib is
	 * used.
	 */
    dbus_conn=conn;

	mainloop = g_main_loop_new(NULL, false);
	/* Set up the DBus connection to work in a GLib event loop */
	dbus_connection_setup_with_g_main(conn, NULL);
	/* Start the glib event loop */
	g_main_loop_run(mainloop);

}