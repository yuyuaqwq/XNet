
#include <Windows.h>

#include <stdio.h>
#include "XNet.h"

#pragma comment(lib, "npcap/Lib/wpcap.lib")
#pragma comment(lib, "npcap/Lib/Packet.lib")


int main()
{
    XNetInit();

    while (1) {
        XNetPoll();
        Sleep(1);
    }

    return 0;
}

