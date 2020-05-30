/*
 * @Author: Bruce.hufy 
 * @Date: 2020-05-30 20:32:43 
 * @Last Modified by:   Bruce.hufy 
 * @Last Modified time: 2020-05-30 20:32:43 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <errno.h> 
#include <netdb.h> 
#include <stdarg.h> 
#include <string.h> 
#include <sys/time.h>
#include <cmath>
/* You will to add includes here */


// Included to get the support library
#include "calcLib.c"

#include "protocol.h"
#define SERV_PORT   8000  
#define BUFF_LEN 512

using namespace std;

/* Needs to be global, to be rechable by callback and main */
int loopCount=0;
int terminate=0;
int flag=0;
int time_out = 1000;

/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum){
  // As anybody can call the handler, its good coding to check the signal number that called it.
  
  printf("Long time no response!\n");
  terminate = 1;
  flag=0;
  exit(1);
  
  return;
}


int main(int argc, char *argv[]){
  
  /* Do more magic */
  /* sock_fd --- socket文件描述符 创建udp套接字*/  
  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock_fd < 0)  
  {  
    perror("socket");  
    exit(1);  
  }  
  
  /* 将套接字和IP、端口绑定 */  
  struct sockaddr_in addr_serv;  
  int len;  
  memset(&addr_serv, 0, sizeof(struct sockaddr_in));  //每个字节都用0填充
  addr_serv.sin_family = AF_INET;  //使用IPV4地址
  addr_serv.sin_port = htons(SERV_PORT);  //端口
  /* INADDR_ANY表示不管是哪个网卡接收到数据，只要目的端口是SERV_PORT，就会被该应用程序接收到 */  
  addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);  //自动获取IP地址
  len = sizeof(addr_serv);  
    
  /* 绑定socket */  
  if(bind(sock_fd, (struct sockaddr *)&addr_serv, sizeof(addr_serv)) < 0)  
  {  
    perror("bind error:");  
    exit(1);  
  }  
  setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&time_out, sizeof(time_out));
    
  int recv_num;  
  int send_num;  
  char send_buf[BUFF_LEN];  
  char recv_buf[BUFF_LEN];  
  struct sockaddr_in addr_client; 
  struct calcMessage calcMessage; 
  struct calcProtocol calcProtocol;
  struct calcProtocol calcProtocol_Res;
  
  
  while(terminate==0)  
  {  
    printf("server wait:\n");      
    struct itimerval alarmTime;
    alarmTime.it_interval.tv_sec=10;
    alarmTime.it_interval.tv_usec=10;
    alarmTime.it_value.tv_sec=10;
    alarmTime.it_value.tv_usec=10;

    /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
    signal(SIGALRM, checkJobbList);
    setitimer(ITIMER_REAL,&alarmTime,NULL); // Start/register the alarm. 
    
    recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);  
    
    if(recv_num < 0)  
    {  
      perror("recvfrom error:");  
      loopCount++;
      printf("LoopCount: %d\n", loopCount);
    }  

    else if (recv_num == sizeof(calcMessage))
    {
      printf("收到了Message\n");
      memcpy(&calcMessage, recv_buf, sizeof(calcMessage)+1);
      printf("Receive calcMessage from client:\n");
      printf("calcMessage type: %d, message: %d, protocal: %d, major_version: %d, minor_version: %d\n\n",
      calcMessage.type,calcMessage.message,calcMessage.protocol,calcMessage.major_version,calcMessage.minor_version);

       /* Initialize the library, this is needed for this library. */
      initCalcLib();
      char *ptr;
      ptr=randomType(); // Get a random arithemtic operator. 

      double f1,f2,fresult;
      int i1,i2,iresult;

      if(ptr[0]=='f'){
        printf("Random create float\n");
        f1=randomFloat();
        f2=randomFloat(); 
        calcProtocol.flValue1=f1;
        calcProtocol.flValue2=f2;
        if(strcmp(ptr,"fadd")==0){
          fresult=f1+f2;
          calcProtocol.arith=5;
          } else if (strcmp(ptr, "fsub")==0){
            fresult=f1-f2;
            calcProtocol.arith=6;
          } else if (strcmp(ptr, "fmul")==0){
            fresult=f1*f2;
            calcProtocol.arith=7;
          } else if (strcmp(ptr, "fdiv")==0){
            fresult=f1/f2;
            calcProtocol.arith=8;
          }
      }else {
        printf("Random create Int\n");
        i1=randomInt();
        i2=randomInt();
        calcProtocol.inValue1=i1;
        calcProtocol.inValue2=i2;
        if(strcmp(ptr,"add")==0){
          iresult=i1+i2;
          calcProtocol.arith=1;
        } else if (strcmp(ptr, "sub")==0){
          iresult=i1-i2;
          calcProtocol.arith=2;
        } else if (strcmp(ptr, "mul")==0){
          iresult=i1*i2;
          calcProtocol.arith=3;
        } else if (strcmp(ptr, "div")==0){
          iresult=i1/i2;
          calcProtocol.arith=4;
        }
      }
      calcProtocol.id = flag;
      send_num = sendto(sock_fd, (char *)&calcProtocol, sizeof(calcProtocol), 0, (struct sockaddr *)&addr_client, len);  
      flag++;
      if(send_num < 0)  
      {  
        perror("sendto error:");  
        exit(1);  
      }
    }

    else if (recv_num == sizeof(calcProtocol))
    {
      printf("收到了Protocol\n");
      memcpy(&calcProtocol_Res,recv_buf,sizeof(calcProtocol_Res));
      if ((calcProtocol_Res.id+1) != flag)
      {
        printf("Received wrong id!\n");
        printf("Correct id is:%d\n",flag-1);
        calcMessage.type = 3;
        calcMessage.message = 2;
        sendto(sock_fd, (char *)&calcMessage, sizeof(calcMessage), 0, (struct sockaddr *)&addr_client, len);  
        break;
      }
      printf("Now id is %d\n",flag-1);
      double fresult;
      int iresult;
      switch (calcProtocol_Res.arith)
      {
      case 1:
        iresult = calcProtocol_Res.inValue1 + calcProtocol_Res.inValue2;
        break;
      case 2:
        iresult = calcProtocol_Res.inValue1 - calcProtocol_Res.inValue2;
        break;
      case 3:
        iresult = calcProtocol_Res.inValue1 * calcProtocol_Res.inValue2;
        break;
      case 4:
        iresult = calcProtocol_Res.inValue1 / calcProtocol_Res.inValue2;
        break;
      case 5:
        fresult = calcProtocol_Res.flValue1 + calcProtocol_Res.flValue2;
        break;
      case 6:
        fresult = calcProtocol_Res.flValue1 - calcProtocol_Res.flValue2;
        break;
      case 7:
        fresult = calcProtocol_Res.flValue1 * calcProtocol_Res.flValue2;
        break;
      case 8:
        fresult = calcProtocol_Res.flValue1 / calcProtocol_Res.flValue2;
        break;
      }

      if (calcProtocol_Res.arith>=1&&calcProtocol_Res.arith<=4)
      {
        if ((calcProtocol_Res.inResult-iresult)<=0.1&&abs(round(calcProtocol_Res.inResult)-calcProtocol_Res.inResult)<=0.000001)
        {
          printf("The answer is %d\n",calcProtocol_Res.inResult);
          printf("OK\n\n");
          calcMessage.message=1;
        }
        else{
          printf("The answer is %d\n",calcProtocol_Res.inResult);
          printf("NOT OK\n\n");
          calcMessage.message=2;
        }
      }
      else{
        if (calcProtocol_Res.flResult-fresult<=1)
        {
          printf("The answer is %8.8g\n",calcProtocol_Res.flResult);
          printf("OK\n\n");
          calcMessage.message=1;
        }
        else{
          printf("The answer is %8.8g\n",calcProtocol_Res.flResult);
          printf("NOT OK\n\n");
          calcMessage.message=2;
        }
      }
      send_num = sendto(sock_fd, (char *)&calcMessage, sizeof(calcMessage), 0, (struct sockaddr *)&addr_client, len);        
    }

    else
    {
      printf("不接收此类信息！\n");
      calcMessage.type = 2;
      calcMessage.message = 2;
      calcMessage.major_version = 1;
      calcMessage.minor_version = 0;
      sendto(sock_fd, (char *)&calcMessage, sizeof(calcMessage), 0, (struct sockaddr *)&addr_client, len);
      exit(1);
    }
    loopCount=0;
    terminate=0;
  }  
   /* 
     Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted, it will be a single alarm 10s after it has been set. 
  */

  // }
  // sleep(2);
  // close(sock_fd);  
  printf("done.\n");
  return 0;  
}
