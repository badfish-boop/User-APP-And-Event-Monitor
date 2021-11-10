#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> /* for glib main loop */
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "dbus_server.h"
#include "login_monitor.h"
#include "dpkg_monitor.h"
#include "systime_monitor.h"
#include "list.h"


const DBusObjectPathVTable server_vtable = {
	.message_function = server_message_handler
};

pthread_mutex_t mutex;
Para para;

void resource_recovery(int sig)
{
    printf(" enter resource_recovery \n");
    exit(sig);
}

int main(void)
{
    DBusConnection *conn;
	DBusError err;
    pthread_t id;
    int ret=0;
    int rv;
    FILE *fp;

    char *filename="/var/log/wtmp";

    signal(SIGTERM,resource_recovery);
    signal(SIGKILL,resource_recovery);
    signal(SIGINT,resource_recovery);

    dbus_error_init(&err);//将错误对象连接到dbus

    // 初始化互斥锁
        if (pthread_mutex_init(&mutex, NULL) != 0){
            // 互斥锁初始化失败
            return 1;
        }

    if ((fp = fopen(filename, "r")) == NULL)
    {
		printf(_("cannot open %s\n"), filename);
        goto out;
    }
    memset(&para,0,sizeof(para));
    linkList *infolist=create_linklist();  //创建链表
    para.infolist=infolist;
    para.fp=fp;
    linklist_insert_head(infolist,NULL);   //插入链表头结点

    ret=pthread_create(&id,NULL,(void *) wtmp_monitor_thread,&para);  //创建wtmp文件监控线程
    if(ret!=0)
    {
        printf ("Create wtmp_monitor_thread error!\n");
        goto out;
    }

    ret=pthread_create(&id,NULL,(void *) utmp_monitor_thread,infolist); //创建utmp文件监控线程
    if(ret!=0)
    {
        printf ("Create utmp_monitor_thread error!\n");
        goto out;
    }

    ret=pthread_create(&id,NULL,(void *) dpkg_monitor_thread,NULL);  //创建dpkg事件监控线程
    if(ret!=0)
    {
        printf ("Create dpkg_monitor_thread error!\n");
        goto out;
    }

    // ret=pthread_create(&id,NULL,(void *) systime_monitor_thread,NULL);  //创建系统时间修改事件监控线程
    // if(ret!=0)
    // {
    //     printf ("Create systime_monitor_thread error!\n");
    //     goto out;
    // }

	/* connect to the daemon bus */
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!conn) {
		fprintf(stderr, "Failed to get a system DBus connection: %s\n", err.message);
		goto out;
	}
    rv = dbus_bus_request_name(conn, "com.kylin.intel.edu.uregis", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (rv != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		fprintf(stderr, "Failed to request name on bus: %s\n", err.message);
		exit(1);
	}

    if (!dbus_connection_register_object_path(conn, "/com/kylin/intel/edu/uregis", &server_vtable, NULL)) {
		fprintf(stderr, "Failed to register a object path for 'UserMonitor'\n");
		exit(1);
    }

    dbus_server_loop(conn);


    pthread_mutex_destroy(&mutex);

    return 0;

out:
    printf("exe out\n");
    pthread_mutex_destroy(&mutex);
    dbus_error_free(&err);
    return ret;
}