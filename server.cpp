
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sstream>

#include <iostream>

static void usage()
{
    std::cerr << " Usage : server [-l listener-port]" << std::endl;
    std::cerr << "         listener-port: The port number to which the server must listen." << std::endl;
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

int main(int argc, char *argv[])
{
    int lport = 0;
    int opt;
    bool lFlag = false;
    while((opt = getopt(argc, argv, "l:")) != -1)
    {
        switch(opt)
        {
            case 'l':
            {
                lFlag = true;
                lport = std::atoi(optarg);
            }
                break;
            default:
                usage();
        }
    }

    if(lFlag == false)
        usage();

    int sock;
    char buf [1024];
    socklen_t length;
    struct sockaddr_in server;
    int msgsock;
    int rval;
    uint16_t sum = 0;
    uint32_t count = 0;

    signal(SIGPIPE, SIG_IGN);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("opening stream socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(lport);
    if(bind(sock, (sockaddr*)&server, sizeof(server)))
    {
        perror("binding stream socket");
        exit(1);
    }


    length = sizeof(server);
    if(getsockname(sock, (sockaddr*)&server, &length))
    {
        perror("getting socket name");
        exit(1);
    }
    std::cout << "Socket has port #" << ntohs(server.sin_port) << std::endl;


    listen(sock, 5);
    do {

            struct sockaddr_in from;
            socklen_t from_len = sizeof(from);
            msgsock = accept(sock, (struct sockaddr*)&from, &from_len);

            if(msgsock == -1)
            {
                perror("accept");
            }   
            else
            {

                inet_ntop(from.sin_family, &from.sin_addr, buf, sizeof(buf));
                std::cout << "Accepted connection from " << buf << ", port " << ntohs(from.sin_port) << std::endl;

                do{
                    if((rval = read(msgsock, buf, sizeof(buf)-1)) < 0) 
                        perror("reading stream message");
                    if(rval == 0)
                        std::cout << "Ending connection" << std::endl;
                    else
                    {
                        buf[rval] = '\0';
                        count += rval;
                    }


                } while (rval != 0);
                
                sum += (count - 1) * 128;
                std::ostringstream os;
                os << "Sum: " << sum << " Len: " << count <<"\n";
                std::string str = os.str();
                const char *ch = str.c_str();
                safe_write(msgsock, ch, sizeof(os.str()));
                sum = 0;
                count = 0;
                close(msgsock);
            }
    } while (true);

    return 0;
}
