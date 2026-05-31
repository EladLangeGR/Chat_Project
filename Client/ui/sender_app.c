#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/ipc.h>
#include <sys/msg.h>


#define MAX_MESSAGE_SIZE 512
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
    struct sockaddr_in mc_addr;
    char* mc_ip;
    char* username;
    char message[MAX_MESSAGE_SIZE];
    char chat_message[MAX_CHAT_MESSAGE];

    if (argc != 5)
    {
        printf("Usage: %s <mc_ip> <port> <username> <queue_id>\n", argv[0]);

        return EXIT_FAILURE;
    }

    mc_ip = argv[1];
    port = atoi(argv[2]);
    username = argv[3];
    queue_id = atoi(argv[4]);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock_fd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&mc_addr, 0, sizeof(mc_addr));

    mc_addr.sin_family = AF_INET;
    mc_addr.sin_port = htons(port);
    mc_addr.sin_addr.s_addr = inet_addr(mc_ip);

    if (SendPidToManager(queue_id) != 0)
    {
        printf("Failed sending PID to manager\n");
    }

    printf("Connected to multicast group %s:%d\n", mc_ip, port);

    while (1)
    {
        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            break;
        }

        message[strcspn(message, "\n")] = '\0';

        snprintf(chat_message, sizeof(chat_message), "%s: %s", username, message);

        if (sendto(sock_fd, chat_message, strlen(chat_message) + 1, 0, (struct sockaddr*)&mc_addr, sizeof(mc_addr)) < 0)
        {
            perror("sendto");
        }
    }

    close(sock_fd);

    return EXIT_SUCCESS;
}


static int SendPidToManager(int _queue_id)
{
    Message msg;
    msg.mtype = 1;
    msg.pid = getpid();

    if (msgsnd(_queue_id, &msg, sizeof(msg.pid), 0) < 0)
    {
        perror("msgsnd");
        return -1;
    }

    return 0;
}