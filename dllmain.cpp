// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
using namespace std;

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
        cout << "[Injected DLL] attached\n";
        break;
    case DLL_THREAD_ATTACH:
        cout << "[Injected DLL] thread attached\n";
        break;
    case DLL_THREAD_DETACH:
        cout << "[Injected DLL] thread detached\n";
        break;
    case DLL_PROCESS_DETACH:
        cout << "[Injected DLL] detached\n";
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) void DLLRun(void* data_globals) {
    cout << "[Injected DLL] run called\n";
}