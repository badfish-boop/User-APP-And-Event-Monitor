//gcc -o test.o test.c
//用于监控文件，并防止文件因为delete后重新生成而产生中间失去对文件监控的问题
//现有可以明确遗留问题，数组对内容保留较少，地址分配应进一步扩充
#include <sys/inotify.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

struct tm *localtime( const time_t *time );
/*
char *result[200];
char *name[200];
char *version[200];
char *newversion[200];
*/
char **result;
char **name;
char **version[200];
char **newversion[200];

//分离字符串，通过空格" "
char *split_line_result(char *result)
{
    char *pch;
    pch = strtok(result, " ,.-");
    int i=0;
    while (pch != NULL)
    {
//        printf("%s\n", pch);
        strcpy(result,pch);
        pch = strtok(NULL, " ");
        if(i == 2){
            return pch;
        }
        i++;
    }
    return NULL;
}

char *split_line_name(char *result)
{
    char *pch;
    pch = strtok(result, " ,.-");
    int i=0;
    while (pch != NULL)
    {
//        printf("%s\n", pch);
        strcpy(result,pch);
        pch = strtok(NULL, " ");
        if(i == 3){
            return pch;
        }
        i++;
    }
    return NULL;
}

char *split_line_version(char *result)
{
    char *pch;
    pch = strtok(result, " ,.-");
    int i=0;
    while (pch != NULL)
    {
//        printf("%s\n", pch);
        strcpy(result,pch);
        pch = strtok(NULL, " ");
        if(i == 4){
            return pch;
        }
        i++;
    }
    return NULL;
}

char *split_line_newversion(char *result)
{
    char *pch;
    pch = strtok(result, " ,.-");
    int i=0;
    while (pch != NULL)
    {
//        printf("%s\n", pch);
        strcpy(result,pch);
        pch = strtok(NULL, " ");
        if(i == 5){
            return pch;
        }
        i++;
    }
    return NULL;
}

/*
//获取upgrade、install、remove相关行
int getResult(char ***result,char ***name,char ***version,char ***newversion)
{
    FILE *fp; 
	fp = fopen("/home/qr/status.diff", "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
    char str[200];
    char str1[200];
    char str2[200];
    char str3[200];
    *result = (char **)malloc(sizeof(char * ) * 200);
    *name =  (char **)malloc(sizeof(char * ) * 200);
    *version =  (char **)malloc(sizeof(char * ) * 200);
    *newversion =  (char **)malloc(sizeof(char * ) * 200);
    char *tmp_result;
    tmp_result = (char *)malloc(200); 
    char *tmp_name;
    tmp_name = (char *)malloc(200); 
    char *tmp_version;
    tmp_version = (char *)malloc(200); 
    char *tmp_newversion; 
    tmp_newversion = (char *)malloc(200); 
    int i = 0;
    printf("getline:\n");  
	while ((read = getline(&line, &len, fp)) != -1)
	{
//        printf("line [%d] = %s\n",i ,line); 
        if(strstr(line,"install ") != NULL){
            strcpy(str,line);
            strcpy(str1,line);
            strcpy(str2,line);
            strcpy(str3,line);
            printf("str=%s\n", str);
            printf("str1=%s\n", str1);
            printf("str2=%s\n", str2);
            printf("str3=%s\n", str3);
            tmp_name = split_line_name(str);
            tmp_result = split_line_result(str1);
            tmp_version = split_line_version(str2);
            tmp_newversion = split_line_newversion(str3);
            printf("tmp_name=%s\n", tmp_name);
            printf("tmp_result=%s\n", tmp_result);
            printf("tmp_version=%s\n", tmp_version);
            printf("tmp_newversion=%s\n", tmp_newversion);
            (*result)[i] = tmp_result;
            (*name)[i] = tmp_name;
            (*version)[i] = tmp_version;
            (*newversion)[i] = tmp_newversion;
            printf("line[%d]=%s\n", i, (*result)[i]);
            printf("line-1[%d]=%s\n", i, (*name)[i]);
            printf("line-2[%d]=%s\n", i, (*version)[i]);
            printf("line-3[%d]=%s\n", i, (*newversion)[i]);
            i++;
            printf("getResult-install-OK\n");
        }else
        if(strstr(line,"remove ") != NULL){
            strcpy(str,line);
            strcpy(str1,line);
            strcpy(str2,line);
            strcpy(str3,line);
            printf("str=%s\n", str);
            printf("str1=%s\n", str1);
            printf("str2=%s\n", str2);
            printf("str3=%s\n", str3);
            tmp_name = split_line_name(str);
            tmp_result = split_line_result(str1);
            tmp_version = split_line_version(str2);
            tmp_newversion = split_line_newversion(str3);
            printf("tmp_name=%s\n", tmp_name);
            printf("tmp_result=%s\n", tmp_result);
            printf("tmp_version=%s\n", tmp_version);
            printf("tmp_newversion=%s\n", tmp_newversion);
            (*result)[i] = tmp_result;
            (*name)[i] = tmp_name;
            (*version)[i] = tmp_version;
            (*newversion)[i] = tmp_newversion;
            printf("line[%d]=%s\n", i, (*result)[i]);
            printf("line-1[%d]=%s\n", i, (*name)[i]);
            printf("line-2[%d]=%s\n", i, (*version)[i]);
            printf("line-3[%d]=%s\n", i, (*newversion)[i]);
            i++;
            printf("getResult-remove-OK\n");
        }else 
        if(strstr(line,"upgrade ") != NULL){
            strcpy(str,line);
            strcpy(str1,line);
            strcpy(str2,line);
            strcpy(str3,line);
            printf("str=%s\n", str);
            printf("str1=%s\n", str1);
            printf("str2=%s\n", str2);
            printf("str3=%s\n", str3);
            tmp_name = split_line_name(str);
            tmp_result = split_line_result(str1);
            tmp_version = split_line_version(str2);
            tmp_newversion = split_line_newversion(str3);
            printf("tmp_name=%s\n", tmp_name);
            printf("tmp_result=%s\n", tmp_result);
            printf("tmp_version=%s\n", tmp_version);
            printf("tmp_newversion=%s\n", tmp_newversion);
            (*result)[i] = tmp_result;
            (*name)[i] = tmp_name;
            (*version)[i] = tmp_version;
            (*newversion)[i] = tmp_newversion;
            printf("line[%d]=%s\n", i, (*result)[i]);
            printf("line-1[%d]=%s\n", i, (*name)[i]);
            printf("line-2[%d]=%s\n", i, (*version)[i]);
            printf("line-3[%d]=%s\n", i, (*newversion)[i]);
            i++;
            printf("getResult-upgrade-OK\n");
        }       
    }
//    printf("getResult-OK\n");
    return i;
}
*/

//获取upgrade、install、remove相关行
int getResult_test()
{
    FILE *fp; 
	fp = fopen("/etc/status.diff", "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

    result = (char **)malloc(200*sizeof(char *));
    name = (char **)malloc(200*sizeof(char *));

    int i = 0;
    printf("getline:\n");  
	while ((read = getline(&line, &len, fp)) != -1)
	{
//        printf("line [%d] = %s\n",i ,line); 
        char str[200];
        char str1[200];
        char str2[200];
        char str3[200];
        if(strstr(line,"install ") != NULL){
            strcpy(str,line);
            strcpy(str1,line);
            strcpy(str2,line);
            strcpy(str3,line);
            printf("str=%s\n", str);
            printf("str1=%s\n", str1);
            printf("str2=%s\n", str2);
            printf("str3=%s\n", str3);
            result[i] = strdup(split_line_result(str));
            name[i] = strdup(split_line_name(str1));
            version[i] = strdup(split_line_version(str2));
            newversion[i] = strdup(split_line_newversion(str3));
            printf("line[%d]=%s\n", i, result[i]);
            printf("line-1[%d]=%s\n", i, name[i]);
            printf("line-2[%d]=%s\n", i, version[i]);
            printf("line-3[%d]=%s\n", i, newversion[i]);
            i++;
            dbus_pkgadd_singal_send(*name);
            printf("getResult-install-OK\n");
        }else if(strstr(line,"remove ") != NULL){
            strcpy(str,line);
            strcpy(str1,line);
            strcpy(str2,line);
            strcpy(str3,line);
            printf("str=%s\n", str);
            printf("str1=%s\n", str1);
            printf("str2=%s\n", str2);
            printf("str3=%s\n", str3);
            result[i] = strdup(split_line_result(str));
            name[i] = strdup(split_line_name(str1));
            version[i] = strdup(split_line_version(str2));
            newversion[i] = strdup(split_line_newversion(str3));
            printf("line[%d]=%s\n", i, result[i]);
            printf("line-1[%d]=%s\n", i, name[i]);
            printf("line-2[%d]=%s\n", i, version[i]);
            printf("line-3[%d]=%s\n", i, newversion[i]);
            i++;
            dbus_pkgremove_singal_send(*name);
            printf("getResult-remove-OK\n");
        }else if(strstr(line,"upgrade ") != NULL){
            strcpy(str,line);
            strcpy(str1,line);
            strcpy(str2,line);
            strcpy(str3,line);
            printf("str=%s\n", str);
            printf("str1=%s\n", str1);
            printf("str2=%s\n", str2);
            printf("str3=%s\n", str3);
            result[i] = strdup(split_line_result(str));
            name[i] = strdup(split_line_name(str1));
            version[i] = strdup(split_line_version(str2));
            newversion[i] = strdup(split_line_newversion(str3));
            printf("line[%d]=%s\n", i, result[i]);
            printf("line-1[%d]=%s\n", i, name[i]);
            printf("line-2[%d]=%s\n", i, version[i]);
            printf("line-3[%d]=%s\n", i, newversion[i]);
            i++;
            printf("getResult-upgrade-OK\n");
        }       
    }
    int n;
    for(n = 0;n<i;n++)
    {
        printf("result[%d]=%s\n",n,result[n]);
        printf("name[%d]=%s\n",n,name[n]);
        printf("version[%d]=%s\n",n,version[n]);
        printf("newversion[%d]=%s\n",n,newversion[n]);
    }
//    printf("getResult-OK\n");
    return i;
}

void inotify_loopback(int __fd, const char *__name, uint32_t __mask,struct inotify_event *event){
    inotify_add_watch(__fd, __name, __mask);
    char buf[BUF_LEN];
    time_t now;
    struct tm *time_now;
    while (1) {
        int n = read(__fd, buf, BUF_LEN);
        char* p = buf;
        while (p < buf + n) {
            event = (struct inotify_event*)p;
            uint32_t mask = event->mask;
            time(&now);
            time_now=localtime(&now);
#if 0
            if (mask & IN_ACCESS) {
               printf("File has been accessed\n");
            }
            if (mask & IN_ATTRIB) {
               printf("File meta data changed\n");
            }
            if (mask & IN_CLOSE_WRITE) {
                printf("File closed after write\n");
            }
            if (mask & IN_CLOSE_NOWRITE) {
               printf("File closed after read\n");
            }
            if (mask & IN_DELETE_SELF) {
                printf("File is deleted\n");
            }
            if (mask & IN_MODIFY) {
                printf("File has been modified\n");
            }
            if (mask & IN_MOVE_SELF) {
               printf("File has been moved\n");
            }
            if (mask & IN_OPEN) {
               printf("File has been opened\n");
            }
            if (mask & IN_IGNORED) {
               printf("File monitor has been removed\n");
            }
#endif
            if (
                mask & IN_IGNORED 
                || 
                mask & IN_ATTRIB
                ||
                mask & IN_CLOSE_WRITE
            )  
            {
                system("cp /var/log/dpkg.log /etc/status-new");
                system("diff /etc/status /etc/status-new > /etc/status.diff ");
                int number = getResult_test();
                printf("-------------------------------------------------------------\n");
                printf("getResult(&diff)=%d\n",number);

                printf("----------------------------------end\n");
                system("mv /etc/status-new /etc/status");
            }
            p += sizeof(struct inotify_event) + event->len;
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

    system("cp /var/log/dpkg.log /etc/status");
    

    int inotify_fd = 0;
    struct inotify_event *event = NULL;
    char* pathname = "/var/log/dpkg.log";

    inotify_fd = inotify_init();

    while(1){
        inotify_loopback(inotify_fd, pathname, IN_ALL_EVENTS, event);
        inotify_fd = inotify_init();
    }
    
    return 0;
}
