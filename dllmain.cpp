// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"

static int doodybug_number = 0;
static int drawcount = 0;

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <DirectXMath.h>
using namespace DirectX;
using namespace std;

#include <vector>
#include <map>
#include <iostream>
#include "globals.h"

#include "3DMaths.h"
#include "WindowFetcher.h"
#include "guiInterface.h"
#include "TestObjRender.h"




// forward declarations
LRESULT CALLBACK WndProcHook2(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init_graphics(IDXGISwapChain* swap_chain);

// global data that gets passed through to us through assembly hooks
DLLGLobals globals = {};
extern "C" __declspec(dllexport) void* DLLGlobals() { return &globals; }

// callback related stuff
HWND target_hwnd = 0;
WNDPROC winproc_callback = 0;

// directX info
ID3D11Device1* dx_device = nullptr;
ID3D11DeviceContext1* dx_device_context = nullptr;
IDXGISwapChain* last_swap_chain = nullptr;

// random debugging junk
std::map<void*, int> device_dict;


extern "C" __declspec(dllexport) void DLLRun(IDXGISwapChain* swap_chain, UINT SyncInterval, UINT Flags) {
    // init graphics if we haven't yet, and the other hooks have fetched the ptr
    if (!dx_device_context)
        init_graphics(swap_chain);
    //    if (!globals.last_d3d11DeviceContext) return;
    //    else init_graphics(swap_chain, (ID3D11DeviceContext1*)globals.last_d3d11DeviceContext);
    if (!dx_device) return; // if no dx_device then our init failed, and our code is inoperable

    // for testing purposes
    RegeneratePerspectiveMatrix(target_hwnd);

    device_dict[swap_chain] = true;
    globals.unique_swap_chains = device_dict.size();
    globals.actual_d3d11DeviceContext = dx_device_context;

    ObjRender(dx_device_context);
    DrawGUI(dx_device, dx_device_context, target_hwnd);
}

LRESULT CALLBACK WndProcHook2(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return 0;

    switch (msg) {
    case WM_KEYDOWN:
        if (wparam == VK_RETURN) {
            char buff[100];
            sprintf_s(buff, "number alert: %d", doodybug_number);
            MessageBoxA(0, buff, "WinProc info", 0);
        }
        if (wparam == VK_BACK) {
            char buff[100];
            sprintf_s(buff, "injected draw status: %d", drawcount);
            MessageBoxA(0, buff, "WinProc info", 0);
        }
    case WM_KEYUP:
    case WM_MOVE:
        break;
    case WM_SIZE:
        RegeneratePerspectiveMatrix(target_hwnd);
        if (last_swap_chain && dx_device)
            ReloadViewportStuff(last_swap_chain, dx_device, hwnd);
        break;
    }

    return CallWindowProcA(winproc_callback, hwnd, msg, wparam, lparam);
}



void init_graphics(IDXGISwapChain* swap_chain) {
    last_swap_chain = swap_chain;
    doodybug_number |= 1;
    dx_device = 0;
    dx_device_context = (ID3D11DeviceContext1*)1; // make this thing have a value

    // config the target widnow & set windows event hook
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swap_chain->GetDesc(&swapChainDesc);
    target_hwnd = swapChainDesc.OutputWindow;
    winproc_callback = (WNDPROC)SetWindowLongPtrA(target_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcHook2);
    if (!winproc_callback) {
        MessageBoxA(0, "failed to set the new winproc hook", "WinProc hook failure", 0);
        return;
    }

    // Get the D3D11 device
    ID3D11Device* pD3D11Device = nullptr;
    if (FAILED(swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&pD3D11Device))) {
        MessageBoxA(0, "failed to get directX device from swap chain", "DirectX hook failure", 0);
        return;
    }
    // Get the ID3D11Device1 interface
    if (FAILED(pD3D11Device->QueryInterface(__uuidof(ID3D11Device1), (void**)(&dx_device)))) {
        MessageBoxA(0, "failed to query directX device for version 1", "DirectX hook failure", 0);
        return;
    }

    // Retrieve the device context
    ID3D11DeviceContext* deviceContext;
    dx_device->GetImmediateContext(&deviceContext);
    if (!deviceContext) {
        MessageBoxA(0, "failed to get directX device context from directX device", "DirectX hook failure", 0);
        dx_device = 0;
        return;
    }
    if (FAILED(deviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&dx_device_context)))) {
        MessageBoxA(0, "failed to query directX device context for version 1", "DirectX hook failure", 0);
        dx_device = 0;
        return;
    }


    InitObjRender(swap_chain, dx_device, target_hwnd);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(target_hwnd);
    ImGui_ImplDX11_Init(dx_device, dx_device_context);

    RegeneratePerspectiveMatrix(target_hwnd);

    doodybug_number |= 2;
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