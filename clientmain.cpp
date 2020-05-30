/*
 * @Author: Bruce.hufy 
 * @Date: 2020-05-30 20:32:52 
 * @Last Modified by:   Bruce.hufy 
 * @Last Modified time: 2020-05-30 20:32:52 
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
#include <iostream>

/* You will to add includes here */


// Included to get the support library
#include "calcLib.c"


#include "protocol.h"

#define DEST_PORT 8000   
#define DSET_IP_ADDRESS  "127.0.0.1" 
#define BUFF_LEN 512

int loopCount = 0;

void send_udp_message(int fd,struct calcMessage calcMessage,struct sockaddr_in addr_serv){
  int len;
  printf("Start to send message\n");
  calcMessage.type=22;
  calcMessage.message=0;
  calcMessage.protocol=17;
  calcMessage.major_version=1;
  calcMessage.minor_version=0;

  while (loopCount<3)
  {
    // char recv_buf[BUFF_LEN];
    int send_num;
    len = sizeof(addr_serv);
    //memcpy(&send_buf, calcMessage, BUFF_LEN);
    send_num = sendto(fd, (char *)&calcMessage, sizeof(calcMessage), 0, (struct sockaddr*)&addr_serv, len);
    sleep(2);
    if(send_num<0){
      loopCount++;
      printf("Send message fail! Resend %d times.\n", loopCount);
      sendto(fd, (char *)&calcMessage, sizeof(calcMessage), 0, (struct sockaddr*)&addr_serv, len);
    }
    else{
      printf("Send message successfully\n");
      break;
    }
  }
  if (loopCount==3)
  {
    printf("Long time no response.\t");
    perror("End\n");
    exit(1);
  }
  // memset(send_buf,0,sizeof(send_buf));
  loopCount = 0;
}

void send_udp_message(int fd,struct calcProtocol calcProtocol,struct sockaddr* addr_serv){
  
}

int main(int argc, char *argv[]){
  
  /* Do magic */
  /* socket文件描述符 */  
  int sock_fd; 

  /* 建立udp socket */  
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);  
  if(sock_fd < 0)  
  {  
    perror("socket create fail!");  
    exit(1);  
  }  

  /* 设置address */  
  struct sockaddr_in addr_serv;  
  int len;
  memset(&addr_serv, 0, sizeof(addr_serv));  
  addr_serv.sin_family = AF_INET;  
  addr_serv.sin_addr.s_addr = inet_addr(DSET_IP_ADDRESS);  
  addr_serv.sin_port = htons(DEST_PORT);  
  len = sizeof(addr_serv); 

  struct calcMessage calcMessage;
  struct calcMessage calcMessage_Res;
  struct calcProtocol calcProtocol;
  char recv_buf[BUFF_LEN];

  send_udp_message(sock_fd, calcMessage, addr_serv);

  recvfrom(sock_fd, (char*)&calcProtocol, sizeof(calcProtocol), 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len); 

  switch (calcProtocol.arith)
  {
    case 1:
      printf("Task:\n%d add %d\n",calcProtocol.inValue1,calcProtocol.inValue2);
      calcProtocol.inResult = calcProtocol.inValue1 + calcProtocol.inValue2;
      printf("The answer of client is %d\n",calcProtocol.inResult);
      break;
    case 2:
      printf("Task:\n%d sub %d\n",calcProtocol.inValue1,calcProtocol.inValue2);
      calcProtocol.inResult = calcProtocol.inValue1 - calcProtocol.inValue2;
      printf("The answer of client is %d\n",calcProtocol.inResult);
      break;
    case 3:
      printf("Task:\n%d mul %d\n",calcProtocol.inValue1,calcProtocol.inValue2);
      calcProtocol.inResult = calcProtocol.inValue1 * calcProtocol.inValue2;
      printf("The answer of client is %d\n",calcProtocol.inResult);
      break;
    case 4:
      printf("Task:\n%d div %d\n",calcProtocol.inValue1,calcProtocol.inValue2);
      calcProtocol.inResult = calcProtocol.inValue1 / calcProtocol.inValue2;
      printf("The answer of client is %d\n",calcProtocol.inResult);
      break;
    case 5:
      printf("Task:\n%f fadd %f\n",calcProtocol.flValue1,calcProtocol.flValue2);
      calcProtocol.flResult = calcProtocol.flValue1 + calcProtocol.flValue2;
      printf("The answer of client is %8.8g\n",calcProtocol.flResult);
      break;
    case 6:
      printf("Task:\n%f fsub %f\n",calcProtocol.flValue1,calcProtocol.flValue2);
      calcProtocol.flResult = calcProtocol.flValue1 - calcProtocol.flValue2;
      printf("The answer of client is %8.8g\n",calcProtocol.flResult);
      break;
    case 7:
      printf("Task:\n%f fmul %f\n",calcProtocol.flValue1,calcProtocol.flValue2);
      calcProtocol.flResult = calcProtocol.flValue1 * calcProtocol.flValue2;
      printf("The answer of client is %8.8g\n",calcProtocol.flResult);
      break;
    case 8:
      printf("Task:\n%f fdiv %f\n",calcProtocol.flValue1,calcProtocol.flValue2);
      calcProtocol.flResult = calcProtocol.flValue1 / calcProtocol.flValue2;
      printf("The answer of client is %8.8g\n",calcProtocol.flResult);
      break;
  }
   
  sendto(sock_fd, (char *)&calcProtocol, sizeof(calcProtocol), 0, (struct sockaddr *)&addr_serv, len);

  recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len);
  memcpy(&calcMessage_Res, recv_buf, sizeof(calcMessage_Res));
  if(calcMessage_Res.message==1){
    printf("OK, U R right!\n");
  }
  else if (calcMessage_Res.message==2&&calcMessage_Res.type==3){
    printf("Error!Different id!\n");
  }
  else if (calcMessage_Res.message==2)
  {
    printf("Oops, U R wrong.\n");
  }

  
  close(sock_fd);  
    
  return 0;  

}
