// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include "globals.h"
using namespace std;

DLLGLobals globals = {};
extern "C" __declspec(dllexport) void* DLLGlobals() {
    return &globals;
}

bool has_injected = false;
int WndProcHook(int code, WPARAM wParam, LPARAM lParam) {
    MSG* windows_event = (MSG*)lParam;

    //cout << "[Injected DLL] message intercepted, code: " << windows_event->message << ", params: " << windows_event->wParam << ", " << windows_event->lParam << endl;

    return(CallNextHookEx(NULL, code, wParam, lParam));
}
BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    

    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
        cout << "[Injected DLL] attached\n";
        // then run attach logic
        if (!has_injected) {
            has_injected = true;
            cout << "[Injected DLL] initiating windows event hook\n";

            HHOOK handle = SetWindowsHookExA(WH_GETMESSAGE, (HOOKPROC)WndProcHook, hModule, GetCurrentThreadId());
            if (!handle) cout << "[Injected DLL] failed to hook windows event function\n";
        }
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

extern "C" __declspec(dllexport) void DLLRun() {
    cout << "[Injected DLL] run called, params: " << globals.debug1 << ", " << globals.debug2 << ", " << globals.debug3 << ", " << globals.debug4 << "\n";
}

