#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/ipc.h>
#include <sys/msg.h>


#define MAX_CHAT_MESSAGE 1024


typedef struct Message
{
    long mtype;
    pid_t pid;

} Message;


static int SendPidToManager(int _queue_id);


int main(int argc, char* argv[])
{
    int sock_fd;
    int port;
    int queue_id;

    struct sockaddr_in local_addr;
    struct ip_mreq mc_req;

    char* mc_ip;
    char recv_buffer[MAX_CHAT_MESSAGE];

    if (argc != 5)
    {
        printf("Usage: %s <mc_ip> <port> <username> <queue_id>\n", argv[0]);

        return EXIT_FAILURE;
    }

    mc_ip = argv[1];
    port = atoi(argv[2]);
    queue_id = atoi(argv[4]);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock_fd < 0)
    {
        perror("socket");

        return EXIT_FAILURE;
    }

    memset(&local_addr, 0, sizeof(local_addr));

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0)
    {
        perror("bind");

        close(sock_fd);

        return EXIT_FAILURE;
    }

    mc_req.imr_multiaddr.s_addr = inet_addr(mc_ip);
    mc_req.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mc_req, sizeof(mc_req)) < 0)
    {
        perror("setsockopt");

        close(sock_fd);

        return EXIT_FAILURE;
    }

    if (SendPidToManager(queue_id) != 0)
    {
        printf("Failed sending PID to manager\n");
    }

    printf("Listening on multicast group %s:%d\n", mc_ip, port);

    while (1)
    {
        ssize_t recv_size;

        recv_size = recvfrom(sock_fd, recv_buffer, sizeof(recv_buffer), 0, NULL, NULL);

        if (recv_size < 0)
        {
            perror("recvfrom");

            continue;
        }

        printf("%s\n", recv_buffer);

        fflush(stdout);
    }

    close(sock_fd);

    return EXIT_SUCCESS;
}


static int SendPidToManager(int _queue_id)
{
    Message msg;

    msg.mtype = 2;
    msg.pid = getpid();

    if (msgsnd(_queue_id, &msg, sizeof(msg.pid), 0) < 0)
    {
        perror("msgsnd");
        return -1;
    }

    return 0;
}
