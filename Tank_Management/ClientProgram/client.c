#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h> 
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>

int flag, fd; //Global variables are initialised to 0

int set_serial_conf(void)
{
    struct termios2 tio;

    fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        printf("open failed\n");

        return -1;
    }

    if (ioctl(fd, TCGETS2, &tio) < 0)
    {
        printf("ioctl TCGETS2 failed\n");

        return (-1);
    }

// Set flags for 'raw' operation

    tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tio.c_iflag &= ~(IXON | IXOFF | IXANY);

// CLOCAL causes the system to disregard the DCD signal state.
// CREAD enables reading from the port.     
    tio.c_cflag |= (CLOCAL | CREAD);

// Attempt to set a custom speed.
    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= CBAUDEX;

// tio.c_cflag |= BOTHER;

    tio.c_ispeed = tio.c_ospeed = 9600;

// Set data bits to 8
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;

// Set no parity

    tio.c_cflag &= ~PARENB;

// single stop bit
    tio.c_cflag &= ~CSTOPB;

// no hardware flow control
    tio.c_cflag &= ~CRTSCTS;

    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 10;

    if (ioctl(fd, TCSETS2, &tio) < 0)
    {
        printf("ioctl TCSETS2 failed\n");

        return (-1);
    }

    if (ioctl(fd, TCGETS2, &tio) < 0)
    {
        printf("ioctl TCGETS2 failed\n");

        return (-1);
    }

    printf("speed changed to %d baud\n", tio.c_ospeed);

    return (0);
}

int read_serial_input(float *level)
{
    int i = 0;

    unsigned char c, buf[64];

    memset((void *)buf, 0, 64);

    while (1)
    {
        if (read(fd, &c,

            sizeof(unsigned char)) != sizeof(unsigned char))
        {
            continue;
        }

        buf[i] = c ;

        if (buf[i] == '\n')
        {
            if (!flag)
            {
                flag = 1;

                return (-1);
            }
            else
            {
                sscanf(buf, "%f\n", level);

                return (0);
            }
        }
        else
        {
            i++;
        }
    }
}

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    float level;
    int sockfd, portno, i=0,n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);

    set_serial_conf();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    while (1) {

        if (read_serial_input(&level) != 0) {

            continue;
        }

        bzero(buffer,256);
        sprintf(buffer, "%d 33 %f cm",i++, level);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) 
            error("ERROR writing to socket");
    }
    return 0;
}
