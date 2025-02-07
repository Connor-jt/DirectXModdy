// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include "globals.h"
#include <vector>
using namespace std;

DLLGLobals globals = {};
extern "C" __declspec(dllexport) void* DLLGlobals() { return &globals; }



extern "C" __declspec(dllexport) void DLLRun() {
    auto var2 = GetCurrentThreadId();

    cout << "[Injected DLL] run called, tID: " << var2 << "\n";
    //cout << "[Injected DLL] run called, params: " << globals.debug1 << ", " << globals.debug2 << ", " << globals.debug3 << ", " << globals.debug4 << "\n";
}



int WndProcHook(int code, WPARAM wParam, LPARAM lParam) {
    MSG* event = (MSG*)lParam;

    auto var2 = GetCurrentThreadId();
    cout << "[Injected DLL] event recieved, tID: " << var2 << ", code: " << event->message << "\n";


    return(CallNextHookEx(NULL, code, wParam, lParam));
}
void InitEventsHook() {
    // find the processes main thread (we just get whatever window thread they have open)
    auto EnumWindowsProc = [](HWND hwnd, LPARAM lParam) -> BOOL {
        DWORD windowProcessId;
        DWORD thread_id = GetWindowThreadProcessId(hwnd, &windowProcessId);
        if (windowProcessId == GetCurrentProcessId())
            *(DWORD*)lParam = thread_id;
        return TRUE;};

    DWORD target_thread = 0;
    EnumWindows(EnumWindowsProc, (LPARAM)&target_thread);
    if (!target_thread) { std::cerr << "[Injected DLL] could not find a thread ID from windows of target process.\n"; return;}

    if (!SetWindowsHookExA(WH_GETMESSAGE, (HOOKPROC)WndProcHook, 0, target_thread)) 
           std::cerr << "[Injected DLL] could not set windows hook. tID:" << GetCurrentThreadId() << endl;
    else { std::cout << "[Injected DLL] successfully set hook. tID:" << GetCurrentThreadId() << endl; return; }

    // pause indefinitely, because if this thread closes then for some reason it also closes the hook (and as you can tell with the suspension, this thread has literally no involvement??)
    SuspendThread(GetCurrentThread());
}
BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    if (GetEnvironmentVariableA("DLL_DO_NOT_INIT", 0, 0)) return TRUE;

    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:{
        cout << "[Injected DLL] process attached tID:" << GetCurrentThreadId() << endl;
        HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)InitEventsHook, 0, 0, 0);
        break;}
    case DLL_THREAD_ATTACH:
        cout << "[Injected DLL] thread attached tID:" << GetCurrentThreadId() << endl;
        break;
    case DLL_THREAD_DETACH:
        cout << "[Injected DLL] thread detached tID:" << GetCurrentThreadId() << endl;
        break;
    case DLL_PROCESS_DETACH:
        cout << "[Injected DLL] process detached tID:" << GetCurrentThreadId() << endl;
        break;
    } return TRUE;
}

