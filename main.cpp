#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <semaphore.h>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"
#define MAX_FD 65536
#define MAX_EVENT_NUMBER 10000

extern int addfd(int epollfd,int fd,bool one_shot);
extern int removefd(int epollfd,int fd);

void addsig( int sig, void(*handler )(int), bool restart = true )
{
   struct sigaction sa;
   memset( &sa, '\0', sizeof( sa ) );
   sa.sa_handler = handler;
   if( restart )
   {
       sa.sa_flags |= SA_RESTART;//SA_RESTART：如果一个程序在执行处于阻塞状态的系统调用时，接受到了一个信号，并且我们为
                                 //该信号注册了处理函数，那么系统调用会被中断，而SA_RESTART标志会自动重启被中断的系统调用
   }
   sigfillset( &sa.sa_mask );
   assert( sigaction( sig, &sa, NULL ) != -1 );
}

void show_error( int connfd, const char* info )
{
    printf( "%s", info );
    send( connfd, info, strlen( info ), 0 ); //向socket中写入出错信息
    close( connfd );
}

int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );

    addsig( SIGPIPE, SIG_IGN );  //忽略SIGPIPE信号，有下划线的代表处理函数，没下划线的代表信号
    //SIGPIPE信号的出发事件为：当向一个读端被关闭的管道或socket中写入数据时。
    threadpool< http_conn >* pool = NULL;
    try
    {
        pool = new threadpool< http_conn >;//创建一个线程池对象
    }
    catch( ... )
    {
        return 1;
    }

    http_conn* users = new http_conn[ MAX_FD ]; //定义用户的连接数量
    assert( users );
    int user_count = 0;

    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 );
    struct linger tmp = { 1, 0 };
    //close()立刻返回，但不会发送未发送完成的数据，而是通过一个REST包强制的关闭socket描述符，即强制退出。
    // #include <arpa/inet.h>
    
    // struct linger {
    // 　　int l_onoff;
    // 　　int l_linger;
    // };

    // 1. l_onoff = 0; l_linger忽略
    
    // close()立刻返回，底层会将未发送完的数据发送完成后再释放资源，即优雅退出。
    
     
    
    // 2. l_onoff != 0; l_linger = 0;
    
    // close()立刻返回，但不会发送未发送完成的数据，而是通过一个REST包强制的关闭socket描述符，即强制退出。
    
     
    
    // 3. l_onoff != 0; l_linger > 0;
    
    // close()不会立刻返回，内核会延迟一段时间，这个时间就由l_linger的值来决定。如果超时时间到达之前，
    // 发送完未发送的数据(包括FIN包)并得到另一端的确认，close()会返回正确，socket描述符优雅性退出。否则，
    // close()会直接返回错误值，未发送数据丢失，socket描述符被强制性退出。需要注意的时，如果socket描述符被设置为非堵塞型，
    // 则close()会直接返回值。
    
    setsockopt( listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof( tmp ) );

    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );

    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret >= 0 );

    ret = listen( listenfd, 5 );
    assert( ret >= 0 );

    epoll_event events[ MAX_EVENT_NUMBER ];
    int epollfd = epoll_create( 5 );//参数只是起提示作用
    assert( epollfd != -1 );
    addfd( epollfd, listenfd, false );//把服务器端fd添加到epollfd中
    http_conn::m_epollfd = epollfd;//把epoll文件句柄赋值给http_conn类中的静态变量m_epollfd

    while( true )
    {
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );//等待服务器端fd是否就绪
        if ( ( number < 0 ) && ( errno != EINTR ) )
        {
            printf( "epoll failure\n" );
            break;
        }

        for ( int i = 0; i < number; i++ )
        {
            int sockfd = events[i].data.fd;
            if( sockfd == listenfd )//如果服务器端fd就绪表明有人链接
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );//获取客户端fd
                if ( connfd < 0 )
                {
                    printf( "errno is: %d\n", errno );
                    continue;
                }
                if( http_conn::m_user_count >= MAX_FD )//如果连接数以满，就拒绝连接
                {
                    show_error( connfd, "Internal server busy" );
                    continue;
                }
                
                users[connfd].init( connfd, client_address );//初始化一个连接。会将连接用户的fd添加到epoll_fd中
            }
            else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) )//如果监听的fd中有人断开连接，就关闭这个链接
            {
                users[sockfd].close_conn();
            }
            else if( events[i].events & EPOLLIN ) //如果监听的fd可以读
            {
                if( users[sockfd].read() )
                {
                    pool->append( users + sockfd );
                }
                else
                {
                    users[sockfd].close_conn(); //如果读出错，就关闭连接
                }
            }
            else if( events[i].events & EPOLLOUT ) //如果监听的fd可以写
            {
                if( !users[sockfd].write() )
                {
                    users[sockfd].close_conn();
                }
            }
            else
            {}
        }
    }

    close( epollfd );
    close( listenfd );
    delete [] users;
    delete pool;
    return 0;
}
