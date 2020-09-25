#ifndef _LOGIN_MONITOR_H
#define _LOGIN_MONITOR_H

#include <utmp.h>
#include <sys/types.h>
#include "list.h"

# define _(Text) (Text)

struct wtmp
{
  short int ut_type;		/* Type of login.  */
  pid_t ut_pid;			/* Process ID of login process.  */
  char ut_line[UT_LINESIZE];	/* Devicename.  */
  char ut_id[4];		/* Inittab ID.  */
  char ut_user[UT_NAMESIZE];	/* Username.  */
  char ut_host[UT_HOSTSIZE];	/* Hostname for remote login.  */
  struct exit_status ut_exit;	/* Exit status of a process marked
				   as DEAD_PROCESS.  */
/* The ut_session and ut_tv fields must be the same size when compiled
   32- and 64-bit.  This allows data files and shared memory to be
   shared between 32- and 64-bit applications.  */
#ifdef __WORDSIZE_TIME64_COMPAT32
  int32_t ut_session;		/* Session ID, used for windowing.  */
  struct
  {
    int32_t tv_sec;		/* Seconds.  */
    int32_t tv_usec;		/* Microseconds.  */
  } ut_tv;			/* Time entry was made.  */
#else
  long int ut_session;		/* Session ID, used for windowing.  */
  struct timeval ut_tv;		/* Time entry was made.  */
#endif

  int32_t ut_addr_v6[4];	/* Internet address of remote host.  */
  char __glibc_reserved[20];		/* Reserved for future use.  */
};

struct user_login_info
{
  char *login_user_name_monit;
  char *login_line;
  uid_t login_user_uid;
  short int login_status;
  struct
  {
    int32_t tv_sec;		/* Seconds.  */
    int32_t tv_usec;		/* Microseconds.  */
  } uli_tv;
  struct user_login_info *next;
};

struct dbus_list_users
{
  uid_t uid;
  char user_name[100];
  char object_path[100];
  // char * ob_path;
};

typedef struct para{

    FILE *fp;
    linkList *infolist;
    
}Para;

void wtmp_monitor_thread(Para *para);
void utmp_monitor(linkList *infolist);
void utmp_monitor_thread(linkList *infolist);
int get_user_info(char **login_user_name,int *user_num);

#endif