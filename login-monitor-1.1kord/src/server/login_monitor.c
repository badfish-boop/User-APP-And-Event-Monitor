#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <utmp.h>
#include <poll.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> /* for glib main loop */
#include <sys/types.h>
#include <error.h>
#include <pwd.h>
#include <grp.h>

#include "typedef.h"
#include "login_monitor.h"
#include "dbus_server.h"
#include "list.h"

#define DEBUG_LEVEL  INFO_OUTPUT

#define F_Path "/var/run/utmp"

#define UCHUNKSIZE	16384	/* How much we read at once. */

extern pthread_mutex_t mutex;
//struct user_login_info * my_head=NULL;
linkList *my_list=NULL;
//struct user_login_info * prev,* current_pos,* current_query_a,* current_query_b;
int32_t login_process_time[7]={0};
//static int tty1_login, tty2_login, tty3_login, tty4_login, tty5_login, tty6_login, tty7_login, tty8_login, tty9_login;

int write_log (FILE* pFile, const char *format, ...) {
	va_list arg;
	int done;
 
	va_start (arg, format);
 
	time_t time_log = time(NULL);
	struct tm* tm_log = localtime(&time_log);
	fprintf(pFile, "%04d-%02d-%02d %02d:%02d:%02d ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);
 
	done = vfprintf (pFile, format, arg);
	va_end (arg);
 
	fflush(pFile);
	return done;
}

uid_t get_cur_uid()
{
    int user_num = 0;
    struct dbus_set_para dbus_para;
    struct dbus_list_users list_user[512];
    // ={{0,"",""}};
    char *state = "active";
    int i=0;

    memset(&dbus_para,0,sizeof(struct dbus_set_para));

    //获取当前登录过的用户数量及用户列表
    user_num = dbus_list_users_method_call(list_user);
    printf("user_num = %d \n",user_num);

    //设置Get方法调用dbus接口参数
    dbus_para.server_name = "org.freedesktop.login1";                    
    dbus_para.interface = "org.freedesktop.DBus.Properties";
    dbus_para.method = "Get";

    //遍历登录的用户，调用Get方法，获取properites中state值
    for(;i<user_num;i++)
    {
        dbus_para.object_path = list_user[i].object_path;
        printf("object_path=%s\n",dbus_para.object_path);
        printf("uid=%d\n",list_user[i].uid);
        printf("user_name=%s\n",list_user[i].user_name);
        if(!strcmp(state,dbus_get_method_call(dbus_para)))  //如果某用户state值为active，则该用户为当前使用屏幕用户
        {
            printf("list_user[%d].uid=%d\n",i,list_user[i].uid);
            break;
        }
        
    }

    return list_user[i].uid;
}

uid_t get_current_uid()
{
    uid_t uid=0;
    struct dbus_set_para dbus_para;

    dbus_para.server_name = "org.freedesktop.login1";  
    dbus_para.object_path = "/org/freedesktop/login1/user/self";                  
    dbus_para.interface = "org.freedesktop.DBus.Properties";
    dbus_para.method = "Get";
    uid = dbus_get_login_self_uid(dbus_para);

    return uid;
}

int get_list(linkList *my_list,char **login_user_name,int *user_num)
{
    linknode *pnode = my_list->_phead;
    linknode* cur_pos;
    int i=0;
    if(my_list->_phead==NULL)
    {
        login_user_name[i]="no user login";
        return -1;
    }
    cur_pos=pnode->_next;
    while(cur_pos != NULL)
    {
        login_user_name[i]=cur_pos->user_login_info->login_user_name_monit;
        i++;
        cur_pos = cur_pos->_next;
    }
    *user_num = i;  

    return 0;

}

int get_user_info(char **login_user_name,int *user_num)
{
    int ret = 0;
    ret=get_list(my_list,login_user_name,user_num);
    return ret;

}

//utmp文件监控，将登陆用户信息维护到链表中
void utmp_monitor(linkList *infolist)
{
    printf("enter utmp_monitor ...\n");
    uid_t ruid ;
    struct utmp *p_utent; 
    struct passwd *pwd;
    long t;
    uid_t NO_UID = -1;
    bool login_status = 0;
    char *user_name = "";
    linknodeData *pdata=NULL;
    my_list=infolist;   //将链表指向本地全局变量

    if (pthread_mutex_lock(&mutex) != 0){     //线程上锁
        printf("lock error!\n");
    }
    printf("utmp lock\n");

    setutent();        //移动到文件头

    while((p_utent = getutent())!= NULL)
    {
        t = p_utent->ut_tv.tv_sec;

        if(p_utent->ut_type == USER_PROCESS) 
        {
            if(strncmp(p_utent->ut_line, "tty", 3)==0)
            {   
                
                linknode *pnode = infolist->_phead;    //将节点指向链表头
                
                while(pnode != NULL)                   //遍历链表
                {
              

                    //if(*current_pos->login_user_name_monit==*p_utent->ut_user)
                    if(pnode->user_login_info==NULL)
                    {
                        pnode = pnode->_next;
                        continue;
                    }
                    printf("current_pos->login_user_name_monit=%s \n",pnode->user_login_info->login_user_name_monit);
                    printf("p_utent->ut_user=%s\n",p_utent->ut_user);
                    if(strncmp(pnode->user_login_info->login_user_name_monit,p_utent->ut_user,strlen(pnode->user_login_info->login_user_name_monit))==0)    //遍历utmp文件，判断用户是否已经登录
                    {
                        printf("user %s has login \n",p_utent->ut_user);    
                        login_status=1;          //该用户已登录
                        //break;
                    }
                    pnode = pnode->_next;
                }
 
                if(login_status==1)
                {
                    login_status=0;
                    continue;
                }else
                {                               //该用户未登录，将该用户信息写入链表，并发送dbus信号

                    pdata = (struct _linknodeData *)malloc(sizeof(struct _linknodeData));
                    pdata->login_user_name_monit = (char *)malloc(100*sizeof(char));
                    pdata->login_line = (char *)malloc(100*sizeof(char));
                    user_name = strdup(p_utent->ut_user);
                    pwd=getpwnam(user_name);
                    if(pwd == NULL){
                        printf(" user login monitor failed ,because use name not can be use\n");
                        ruid = NO_UID;
                    }
                    free(user_name);
                    ruid = pwd->pw_uid;
                    pdata->login_user_uid=ruid;
                    if (ruid == NO_UID )
                        printf ("cannot get real UID\n");
                    dbus_userlogin_singal_send(ruid);
                    printf("------USER %s on %s login in %.20s,pid is %d ------\n",p_utent->ut_user,p_utent->ut_line,(ctime(&t) + 4),p_utent->ut_pid);
                    strcpy(pdata->login_user_name_monit,p_utent->ut_user);
                    strcpy(pdata->login_line,p_utent->ut_line);
                    pdata->login_status=true;
                    pdata->uli_tv.tv_sec=p_utent->ut_tv.tv_sec;
                    linklist_append_last(infolist,pdata);          //在链表尾插入当前节点
                    
                }
            }
        }
    }
    pthread_mutex_unlock(&mutex);  //线程解锁
    printf("utmp unlock\n"); 
    endutent();  /* closes the utmp file. */

}


//wtmp文件监控，将登出用户信息从链表中移除
int wtmp_monitor(Para *para)
{
    printf("enter wtmp_monitor\n");

    uid_t ruid ;
    struct wtmp p_wtent; 
    int size=0;
    long t;
    linknode *pre_pos,*cur_pos;
    linkList *infolist;

    if (pthread_mutex_lock(&mutex) != 0){
        printf("lock error!\n");
    }
    printf("wtmp lock\n");
    FILE *fp=para->fp;
    infolist=para->infolist;

    linknode *pnode = infolist->_phead;  //将节点指向链表头
    fseek(fp, 0, SEEK_SET); //将文件指针指向开头

    while(!feof(fp))
    {

        size = fread(&p_wtent,sizeof(p_wtent),1,fp);
        if(size < 0)
        {
            printf("file read err\n");
            return -1;
        }
        t=p_wtent.ut_tv.tv_sec;

        if(p_wtent.ut_type == DEAD_PROCESS) 
        {
            if(strncmp(p_wtent.ut_line, "tty", 3)==0)
            {
                pre_pos = pnode;
                cur_pos = pnode->_next;
                if(infolist->_phead==NULL)  //如果链表为NULL，退出
                    break;
                while(cur_pos != NULL)     //遍历链表
                {

                    //printf("current_pos->login_user_name_monit=%s\n",current_pos->login_user_name_monit);
                    if(cur_pos->user_login_info->login_user_name_monit==NULL)
                    {
                        cur_pos = cur_pos->_next;
                        continue;
                    }
                    
                    //printf("current_pos->login_user_name_monit=%s \n",current_pos->login_user_name_monit);
                    //printf("current_pos->login_line=%s \n",current_pos->login_line); 
                    //printf("p_wtent.ut_line=%s \n",p_wtent.ut_line);
                    
                    if(strncmp(cur_pos->user_login_info->login_line,p_wtent.ut_line,strlen(cur_pos->user_login_info->login_line))==0)  //查找登出的用户是否在链表中有记录
                    {
                        
                        if(p_wtent.ut_tv.tv_sec > cur_pos->user_login_info->uli_tv.tv_sec && cur_pos->user_login_info->login_status==1)  //如果该用户之前正在登录，且登出时间大于登录时间，则认为该用户登出
                        {
                        //printf("current_pos->login_line=%s\n",current_pos->login_line);
                        //printf("p_wtent.ut_line=%s\n",p_wtent.ut_line);
                        //printf("p_wtent.ut_tv.tv_sec=%d\n",p_wtent.ut_tv.tv_sec);
                        //printf("current_pos->uli_tv.tv_sec=%d\n",current_pos->uli_tv.tv_sec);
                        ruid=cur_pos->user_login_info->login_user_uid;
                        cur_pos->user_login_info->login_status=0;
                        printf("------USER %s on %s logout in %.20s,pid is %d ------\n",cur_pos->user_login_info->login_user_name_monit,p_wtent.ut_line,(ctime(&t) + 4),p_wtent.ut_pid);
                        //printf("ruid=%d\n",ruid);
                        dbus_userlogout_singal_send(ruid);
                        pre_pos->_next=cur_pos->_next;                  //在链表中删除该节点
                        free(cur_pos);
                        infolist->_count--;
                        cur_pos=NULL;
                        break;
                        }      
                    }
                    pre_pos=pre_pos->_next;
                    cur_pos=cur_pos->_next;
                }
            }
        }
        if(feof(fp))                               //避免检测到文件结束符后多读一行
        break;
    }
    pthread_mutex_unlock(&mutex);
    printf(" wtmp unlock\n"); 

    return 0;
}

void wtmp_monitor_thread(Para *para)
{
    printf("enter wtmp_monitor_thread...\n");

    int length, offset;
    unsigned char buffer[256];
    struct inotify_event *event;
    struct pollfd fds;

    int inotifyFd = inotify_init();
    if(inotifyFd < 0)
    {
        fprintf(stderr, "inotify init failed\n");
        exit(1);
    }
    //添加要监视的目录 "/var/log/wtmp", 监视 IN_CLOSE_WRITE 事件。
    int err = inotify_add_watch(inotifyFd, "/var/log/wtmp", IN_CLOSE_WRITE );
    if(err < 0)
    {
        fprintf(stderr, "inotify_add_watch add path failed\n");
        close(inotifyFd);
        exit(1);
    }

    fds.fd = inotifyFd;
	fds.events = POLLIN;
	fds.revents = 0;
    
    while(poll(&fds, 1, -1) > -1)
    {

        length = read(inotifyFd, buffer, 256);
        if(length <= 0) break;
        
        offset = 0;
        while(offset <= length)
        {
            event = (struct inotify_event*)(buffer + offset);
            
                if(event->mask & IN_CLOSE_WRITE)
                {
                  wtmp_monitor(para);
                }
            
            offset += sizeof(struct inotify_event) + event->len;
        }
    }
    close(inotifyFd);
    fclose(para->fp);

}

void utmp_monitor_thread(linkList *infolist)
{
    printf("enter utmp_monitor_thread...\n");

    int length, offset;
    unsigned char buffer[256];
    struct inotify_event *event;
    struct pollfd fds;

    int inotifyFd = inotify_init();
    if(inotifyFd < 0) 
    {
        fprintf(stderr, "inotify init failed\n");
        exit(1);
    }
    
    //添加要监视的目录 "/var/log/utmp", 监视 IN_CLOSE_WRITE 事件。
    int err = inotify_add_watch(inotifyFd, "/var/run/utmp", IN_CLOSE_WRITE);
    if(err < 0)
    {
        fprintf(stderr, "inotify_add_watch add path failed\n");
        close(inotifyFd);
        exit(1);
    }

    fds.fd = inotifyFd;
	fds.events = POLLIN;
	fds.revents = 0;

    utmpname(_PATH_UTMP); /* #define _PATH_UTMP "/var/log/utmp" */
    
    while(poll(&fds, 1, -1) > -1)
    {
        length = read(inotifyFd, buffer, 256);
        if(length <= 0) break;
        
        offset = 0;
        while(offset <= length)
        {
            event = (struct inotify_event*)(buffer + offset);
            
                if(event->mask & IN_CLOSE_WRITE)
                {
                    utmp_monitor(infolist); 
                }
            
            offset += sizeof(struct inotify_event) + event->len;
        }
    }
    
    close(inotifyFd);
    endutent(); /* closes the utmp file. */

}