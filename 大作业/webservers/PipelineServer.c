#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>    
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include "helper.h"

int parse_argc(int argc, char **argv);
int init_server(int port);
int do_accept(int listenfd);
void disconnect(int connfd);

// 流水线式Web服务器
int main(int argc, char **argv)
{	
    int port = parse_argc(argc,argv);

    int listenfd = init_server(port);

    while(1){
        int connfd = do_accept(listenfd);

        do_process(connfd);

        disconnect(connfd);
	}
    close(listenfd);
	
	return 0;
}

int parse_argc(int argc, char **argv){
    // 提示使用方式 获取服务器端口和映射目录
	if (argc != 3){
        fprintf(stderr, "usage: %s <port> <path>\n", argv[0]);
		exit(1);
	}
    int port = atoi(argv[1]);
    
    int ret = chdir(argv[2]);
    if(ret == -1){
        perror("chdir error");
        exit(1);
    }
    
    return port;
}

int init_server(int port){
    int listenfd;
    struct sockaddr_in servaddr;
    int ret = -1;

    // 建立listenfd套接字
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        perror("socket error");
        exit(1);
    }

    // 设置属性
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    //设置地址重用
    int opt = 1;
    setsockopt(listenfd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&opt,sizeof(opt));
    
    // bind
	ret = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(ret == -1){
        perror("bind error");
        exit(1);
    }

    // listen
	ret = listen(listenfd,256);
    if(ret == -1){
        perror("listen error");
        exit(1);
    }

    return listenfd;
}

int do_accept(int listenfd){
	int connfd;
	socklen_t clilen;
	struct sockaddr_in	cliaddr;
    
    //accept
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
    if (connfd == -1) {   
        perror("accept error");
        exit(1);
    }
    //打印发起连接的客户端信息
    char str[INET_ADDRSTRLEN];
    printf("New Client: IP %s, PORT %d\n",
        inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
        ntohs(cliaddr.sin_port));

    return connfd;
}

void disconnect(int connfd){
    close(connfd);
}