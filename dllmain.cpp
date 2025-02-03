// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
        MessageBoxA(0, "DLL Alert: Attached!", "Epic", MB_OK);
        break;
    case DLL_THREAD_ATTACH:
        MessageBoxA(0, "DLL Alert: Thread attached!", "Epic", MB_OK);
        break;
    case DLL_THREAD_DETACH:
        MessageBoxA(0, "DLL Alert: Thread detached!", "Sad", MB_OK);
        break;
    case DLL_PROCESS_DETACH:
        MessageBoxA(0, "DLL Alert: Detached!", "Sad", MB_OK);
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) void DLLRun(void* data_globals) {
    MessageBoxA(0, "DLL Alert: DLL has been run!", "Epic", MB_OK);
}