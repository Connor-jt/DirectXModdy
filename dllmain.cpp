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

DLLGLobals globals = {};
extern "C" __declspec(dllexport) void* DLLGlobals() { return &globals; }

HWND target_hwnd = 0;
//DWORD target_thread = 0;

WNDPROC OldWndProc = 0;

ID3D11Device1* dx_device = nullptr;
ID3D11DeviceContext1* dx_device_context = nullptr;
void init_graphics(ID3D11DeviceContext1* _dx_device_context) {
    dx_device_context = _dx_device_context;

    // Get the directX device
    ID3D11Device* pD3D11Device = nullptr;
    dx_device_context->GetDevice(&pD3D11Device);
    if (!pD3D11Device) {
        cout << "[Injected DLL] failed 1\n";
        MessageBoxA(0, "failed to get directX device", "DirectX hook failure", 0);
        return;}
    // Get the ID3D11Device1 interface
    if (FAILED(pD3D11Device->QueryInterface(__uuidof(ID3D11Device1), (void**)(&dx_device)))) {
        cout << "[Injected DLL] failed 2\n";
        MessageBoxA(0, "failed to query directX device for version 1", "DirectX hook failure", 0);
        return;}


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(target_hwnd);
    ImGui_ImplDX11_Init(dx_device, dx_device_context);
}

LRESULT CALLBACK WndProcHook2(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
extern "C" __declspec(dllexport) void DLLRun(IDXGISwapChain1* swap_chain, UINT SyncInterval, UINT Flags) {
    //cout << "[Injected DLL] device: " << dx_device << " device context: " << dx_device_context << " global context: " << globals.last_d3d11DeviceContext << "\n";
    //cout << "[Injected DLL] globals ptr: " << &globals << "\n";
    // fail if bad swapchain
    if (!swap_chain) {
        cout << "[Injected DLL] no swap chain\n";
        return;
    }
    cout << "[Injected DLL] hwnd: " << target_hwnd << "\n";
    if (!target_hwnd) {
        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        swap_chain->GetDesc(&swapChainDesc);
        target_hwnd = swapChainDesc.OutputWindow;
        OldWndProc = (WNDPROC)SetWindowLongPtrA(target_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcHook2);
        cout << "[Injected DLL] hwnd: " << target_hwnd << " swap chain: " << swap_chain << "\n";
    }
    // fail run if no window handle
    //if (!target_hwnd || !target_thread) {
    //    cout << "[Injected DLL] hwnd: " << target_hwnd << " thread: " << target_thread << "\n";
    //    return;
    //}

    // init graphics if we haven't yet, and the other hooks have fetched the ptr
    if (!dx_device_context) {
        if (globals.last_d3d11DeviceContext)
            init_graphics((ID3D11DeviceContext1*)globals.last_d3d11DeviceContext);
        else {
            cout << "[Injected DLL] no device context yet.\n";
            return;
        }
    }

    if (!dx_device) {
        cout << "[Injected DLL] failed 3\n";
        return; // possible due to error
    }


    //cout << "[Injected DLL] rendering 1\n";
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    //cout << "[Injected DLL] rendering 2\n";
    ImGui::Begin("Another Window"); 
    ImGui::Text("Hello from another window!");
    //cout << "[Injected DLL] rendering 3\n";
    ImGui::End();

    //cout << "[Injected DLL] rendering 4\n";
    ImGui::Render();
    //cout << "[Injected DLL] rendering 5\n";
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    //cout << "[Injected DLL] rendering 6\n";

    RECT rect;
    if (GetWindowRect(target_hwnd, &rect)){
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        cout << "[Injected DLL] run called, tID: " << GetCurrentThreadId() << " width: " << width << " height: " << height << "\n";
    }
    else {
        cout << "[Injected DLL] run called, tID: " << GetCurrentThreadId() << "\n";
    }

    //cout << "[Injected DLL] run called, params: " << globals.debug1 << ", " << globals.debug2 << ", " << globals.debug3 << ", " << globals.debug4 << "\n";
}



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcHook2(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return 0;

    switch (msg) {
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_MOVE:
        break;
    }

    return CallWindowProcA(OldWndProc, hwnd, msg, wparam, lparam);
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    //if (GetEnvironmentVariableA("DLL_DO_NOT_INIT", 0, 0)) return TRUE;
    //switch (ul_reason_for_call){
    //case DLL_PROCESS_ATTACH:
    //case DLL_THREAD_ATTACH:
    //case DLL_THREAD_DETACH:
    //case DLL_PROCESS_DETACH:
    //} return TRUE;
}