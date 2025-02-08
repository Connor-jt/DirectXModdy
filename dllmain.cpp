// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


#include <vector>
#include <iostream>
#include "globals.h"


using namespace std;

// forward declarations
LRESULT CALLBACK WndProcHook2(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// global data that gets passed through to us through assembly hooks
DLLGLobals globals = {};
extern "C" __declspec(dllexport) void* DLLGlobals() { return &globals; }

// callback related stuff
HWND target_hwnd = 0;
WNDPROC winproc_callback = 0;

// directX info
ID3D11Device1* dx_device = nullptr;
ID3D11DeviceContext1* dx_device_context = nullptr;

void init_graphics(IDXGISwapChain* swap_chain, ID3D11DeviceContext1* _dx_device_context) {
    dx_device_context = _dx_device_context;

    // config the target widnow & set windows event hook
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swap_chain->GetDesc(&swapChainDesc);
    target_hwnd = swapChainDesc.OutputWindow;
    winproc_callback = (WNDPROC)SetWindowLongPtrA(target_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcHook2);
    if (!winproc_callback){
        MessageBoxA(0, "failed to set the new winproc hook", "WinProc hook failure", 0);
        return;}
    
    // Get the directX device
    ID3D11Device* pD3D11Device = nullptr;
    dx_device_context->GetDevice(&pD3D11Device);
    if (!pD3D11Device) {
        MessageBoxA(0, "failed to get directX device", "DirectX hook failure", 0);
        return;}
    // Get the ID3D11Device1 interface
    if (FAILED(pD3D11Device->QueryInterface(__uuidof(ID3D11Device1), (void**)(&dx_device)))) {
        MessageBoxA(0, "failed to query directX device for version 1", "DirectX hook failure", 0);
        return;}

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(target_hwnd);
    ImGui_ImplDX11_Init(dx_device, dx_device_context);
}

extern "C" __declspec(dllexport) void DLLRun(IDXGISwapChain* swap_chain, UINT SyncInterval, UINT Flags) {
    // init graphics if we haven't yet, and the other hooks have fetched the ptr
    if (!dx_device_context)
        if (!globals.last_d3d11DeviceContext) return;
        else init_graphics(swap_chain, (ID3D11DeviceContext1*)globals.last_d3d11DeviceContext);
    

    if (!dx_device) {
        MessageBoxA(0, "no directX device", "DirectX hook error", 0);
        return;}


    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Another Window"); 
    ImGui::Text("Hello from another window!");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}



LRESULT CALLBACK WndProcHook2(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return 0;

    switch (msg) {
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_MOVE:
        break;
    }

    return CallWindowProcA(winproc_callback, hwnd, msg, wparam, lparam);
}

//BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    //if (GetEnvironmentVariableA("DLL_DO_NOT_INIT", 0, 0)) return TRUE;
    //switch (ul_reason_for_call){
    //case DLL_PROCESS_ATTACH:
    //case DLL_THREAD_ATTACH:
    //case DLL_THREAD_DETACH:
    //case DLL_PROCESS_DETACH:
    //} return TRUE;
//}