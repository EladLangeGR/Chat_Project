#include <stdio.h>
#include <stdlib.h>
#include "server_net.h"
#include "server_mng.h"

#define SERVER_PORT 2222

int main(void)
{
    ServerMng* mng;
    ServerNet* net;

    mng = ServerMng_Create();
    if (!mng) {
        fprintf(stderr, "[Main] Failed to create ServerMng\n");
        return 1;
    }

    net = ServerNet_Create(SERVER_PORT,
                           ServerMng_OnNewClient,
                           ServerMng_OnGotMsg,
                           ServerMng_OnCloseClient,
                           ServerMng_OnFail,
                           mng);
    if (!net) {
        fprintf(stderr, "[Main] Failed to create ServerNet\n");
        ServerMng_Destroy(&mng);
        return 1;
    }

    printf("[Main] Server started on port %d\n", SERVER_PORT);

    ServerNet_Run(net); /* blocking – returns when StopRun is called */

    ServerNet_Destroy(&net);
    ServerMng_Destroy(&mng);

    printf("[Main] Server shut down cleanly\n");
    return 0;
}
