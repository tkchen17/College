
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream>


static void usage()
{
    std::cerr << " Usage : server [-s server-ip] server-port" << std::endl;
    std::cerr << "         server-ip: Specify the server’s IPv4 number in dotted-quad format." << std::endl;
    std::cerr << "         server-port: The server port number to which the client must connect." << std::endl;
    exit (1);
}

static ssize_t safe_write(int fd, const char *buf, size_t len)
{
    while(len>0)
    {
        ssize_t wlen = write(fd, buf, len);
        if(wlen == -1)
            return -1;
        len -= wlen;
        buf += wlen;
    }
    return len;
}

static int print_response(int fd)
{
    char buf[1024];
    int rval = 1;
    while(rval > 0)
    {
        if((rval = read(fd, buf, sizeof(buf)-1)) == -1)
        {
            perror("reading stream message");
            return -1;
        }
        else if(rval > 0)
        {
            std::cout << buf;
        }

        
    }

    return 0;
}

int main(int argc, char *argv[])
{
    const char *dip = "127.0.0.1"; 
    int lport = 0;                 
    int opt;
    bool sFlag = false;
    while((opt = getopt(argc, argv, "s:")) != -1)
    {
        switch(opt)
        {
            case 's':
            {
                sFlag = true;
            }
                break;
            default:
                usage();
        }
    }

    int sock;
    struct sockaddr_in server;


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("opening stream socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    if(sFlag == true)
    {
        dip = argv[2];
        lport = atoi(argv[3]);
    }
    else
    {
        lport = atoi(argv[1]);
    }

    if(inet_pton(AF_INET, dip, &server.sin_addr) <= 0)
    {
        std::cerr << "Invalid address/format" << argv[2] <<std::endl;
        exit(2);
    }

    server.sin_port = htons(lport);

    if(connect(sock, (sockaddr*)&server, sizeof(server)) < 0)
    {
        perror("connecting stream socket");
        exit(1);
    }

    char buf[2048];
    ssize_t len;
    do{
        len = read(fileno(stdin), buf, sizeof(buf)-1);

        if(safe_write(sock, buf, len) < 0)
            perror("writing on stream socket");
    }while(len != 0);

    shutdown(sock, SHUT_WR);

    print_response(sock);

    close(sock);

    return 0;
}
