#include <stdio.h>

#include "client_mng.h"
#include "ui.h"


int main()
{
    if (ClientMngInit() != CM_SUCCESS)
    {
        printf("Failed to initialize client manager\n");

        return 1;
    }

    UIRun();

    ClientMngDestroy();

    return 0;
}