//gcc -o test.o test.c
//用于监控文件
#include <sys/inotify.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <libgen.h>
#include <dirent.h>
#include <errno.h>
#include "dbus_server.h"

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define EVENT_NUM 12
char *event_str[EVENT_NUM] =
{
 "IN_ACCESS",
 "IN_MODIFY",
 "IN_ATTRIB",
 "IN_CLOSE_WRITE",
 "IN_CLOSE_NOWRITE",
 "IN_OPEN",
 "IN_MOVED_FROM",
 "IN_MOVED_TO",
 "IN_CREATE",
 "IN_DELETE",
 "IN_DELETE_SELF",
 "IN_MOVE_SELF"
};
int readFileList(int fd,char *basePath) //初始化监控一级已有子目录
{
     DIR *dir;
     struct dirent *ptr;
     char base[1024];
 
     if ((dir=opendir(basePath)) == NULL)
     {
         perror("Open dir error...");
         exit(1);
     }
 
     while ((ptr=readdir(dir)) != NULL)
     {
         if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
             continue;
         else if(ptr->d_type == 4)    ///dir
         {
			 memset(base,'\0',sizeof(base));
             strcpy(base,basePath);
             strcat(base,"/");
             strcat(base,ptr->d_name);
             inotify_add_watch(fd, base, IN_CREATE|IN_DELETE);
         }
     }
     closedir(dir);
     return 0;
}
int inotify_call(char *path)
{

	 int fd;
	 int wd;
	 int len;
	 int nread;
	 char buf[BUFSIZ];
	 char ch[1024]={0};
	 memset(ch,0,1024);
	 struct inotify_event *event;
	 int i;

	 /*if(argc < 2)
	 {
	  fprintf(stderr, "%s path\n", argv[0]);
	  return -1;
	 }*/

	 fd = inotify_init();
	 if( fd < 0 )
	 {
		  fprintf(stderr, "inotify_init failed\n");
		  return -1;
	 }

	 wd = inotify_add_watch(fd, path, IN_CREATE|IN_DELETE);
	 if(wd < 0)
	 {
		  perror(path);
	 }
	 readFileList(fd,path);
	 buf[sizeof(buf) - 1] = 0;
	 while( (len = read(fd, buf, sizeof(buf) - 1)) > 0 )
	 {
		  nread = 0;
		  while( len > 0 )
		  {
			   event = (struct inotify_event *)&buf[nread];
			   for(i=0; i<EVENT_NUM; i++)
			   {
					if((event->mask >> i) & 1)
					{
						     
							 if(event_str[i] == event_str[8])
							 {
	
									char *detect = NULL;
									detect = strstr(event->name,".dpkg-new");
									if(detect != NULL)
									{
										char substring[1024] = {0};
										char *subdetect = NULL;
										memset(substring,0,1024);
										strncpy(substring,event->name, detect-event->name);
										subdetect = strstr(substring,".");
										if(subdetect == NULL)
										{
											char subpath[1024] = {0};
											memset(subpath,'\0',sizeof(subpath));
											strcpy(subpath,path);
											strcat(subpath,"/");
											strcat(subpath, substring);
											usleep(15);
											wd = inotify_add_watch(fd, subpath, IN_CREATE|IN_DELETE);
											if(wd < 0)
											{
												  perror(subpath);
												  return -1;
											}
										}
									}
							}
							if(event_str[i] == event_str[8])
							{
									char *ret=NULL;
									ret = strstr(event->name, ".desktop");
								  	if(ret !=NULL)
									{
										char ch1[1024]={0};
										memset(ch1,0,1024);
										strncpy(ch1,event->name, ret-event->name);
										strcpy(ch,ch1);
										dbus_pkgadd_singal_send(ch);
									}
			

							 }
							  else if (event_str[i] == event_str[9])
						      {
									 char *ret = NULL;
									 ret = strstr(event->name, ".desktop");
								  	 if(ret != NULL)
									 {
										 char ch1[1024]={0};
										 memset(ch1,0,1024);
										 strncpy(ch1,event->name, ret-event->name);
										 strcpy(ch,ch1);
										 dbus_pkgremove_singal_send(ch);
									 }
							  }
						      
					} 
			   }
			   nread = nread + sizeof(struct inotify_event) + event->len;
			   len = len - sizeof(struct inotify_event) - event->len;
		  }
	 }
}


int CreateDir(const char *sPathName)  
  {  
      char DirName[256];  
      strcpy(DirName, sPathName);  
      int i,len = strlen(DirName);
      for(i=1; i<len; i++)  
      {  
          if(DirName[i]=='/')  
          {  
              DirName[i] = 0; 
              if(access(DirName, NULL)!=0)  
              {  
                  if(mkdir(DirName, 0755)==-1)  
                  {   
                      printf("mkdir   error\n");   
                      return -1;   
                  }  
              }  
              DirName[i] = '/';  

          }  
      }  

      return 0;  
} 


int dpkg_monitor_thread() {

    printf("enter dpkg_monitor_thread...\n");

    inotify_call("/usr/share/applications");
    
    return 0;
}
