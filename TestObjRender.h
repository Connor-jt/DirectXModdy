#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// other stuff


ID3D11VertexShader* __vertexShader;
ID3D11PixelShader* __pixelShader;

ID3D11InputLayout* __inputLayout;

ID3D11Buffer* __vertexBuffer;
ID3D11Buffer* __indexBuffer;
UINT __numIndices;
UINT __stride;
UINT __offset;

ID3D11SamplerState* __samplerState;
ID3D11ShaderResourceView* __textureView;

ID3D11Buffer* __constantBuffer;

// bonus other stuff
ID3D11RasterizerState* SolidRasterState = nullptr;
ID3D11DepthStencilState* SolidDepthStencilState = nullptr;

// even more bonus other stuff
D3D11_VIEWPORT viewport;
ID3D11RenderTargetView* d3d11RenderTargetView;
ID3D11Texture2D* depthStencilBuffer;
ID3D11DepthStencilView* depthStencilView;

struct __Constants {
    float4x4 modelViewProj;
};


void ReloadViewportStuff(IDXGISwapChain* swap_chain, ID3D11Device1* dx_device, HWND hwnd) {
    if (d3d11RenderTargetView) d3d11RenderTargetView->Release();

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT)); // not needed??
    viewport.Width = clientRect.right - clientRect.left;
    viewport.Height = clientRect.bottom - clientRect.top;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;

    ID3D11Texture2D* backbuffer;
    HRESULT hResult = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer);
    assert(SUCCEEDED(hResult));
    hResult = dx_device->CreateRenderTargetView(backbuffer, nullptr, &d3d11RenderTargetView);
    assert(SUCCEEDED(hResult));
    backbuffer->Release();
}

void InitObjRender(IDXGISwapChain* swap_chain, ID3D11Device1* dx_device, HWND hwnd) {
    // Create Vertex Shader

    //cout << "init STARTED" << endl;
    ID3DBlob* vsBlob;
    {
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"D:\\Projects\\VS\\DirectXModdy\\injected_resources\\injected_shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
        if (FAILED(hResult))
        {
            const char* errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if (shaderCompileErrorsBlob) {
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return;
        }

        hResult = dx_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &__vertexShader);
        assert(SUCCEEDED(hResult));
    }

    // Create Pixel Shader
    {
        ID3DBlob* psBlob;
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"D:\\Projects\\VS\\DirectXModdy\\injected_resources\\injected_shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
        if (FAILED(hResult))
        {
            const char* errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if (shaderCompileErrorsBlob) {
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return;
        }

        hResult = dx_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &__pixelShader);
        assert(SUCCEEDED(hResult));
        psBlob->Release();
    }

    // Create Input Layout
    {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0  },
        };

        HRESULT hResult = dx_device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &__inputLayout);
        assert(SUCCEEDED(hResult));
        vsBlob->Release();
    }

    // Create Vertex and Index Buffer
    {
        float vertexData[] = {
            //  pos_x, pos_y, pos_z,    ux_x, uv_y
                -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
                -0.5f, -0.5f,  0.5f,    0.0f, 1.0f,
                -0.5f,  0.5f, -0.5f,    1.0f, 0.0f,
                -0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
                 0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
                 0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
                 0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
                 0.5f,  0.5f,  0.5f,    0.0f, 0.0f
        };

        uint16_t indices[] = {
            0, 6, 4,
            0, 2, 6,
            0, 3, 2,
            0, 1, 3,
            2, 7, 6,
            2, 3, 7,
            4, 6, 7,
            4, 7, 5,
            0, 4, 5,
            0, 5, 1,
            1, 5, 7,
            1, 7, 3
        };
        __stride = 5 * sizeof(float);
        __offset = 0;
        __numIndices = sizeof(indices) / sizeof(indices[0]);

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = sizeof(vertexData);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { vertexData };

        HRESULT hResult = dx_device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &__vertexBuffer);
        assert(SUCCEEDED(hResult));

        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = sizeof(indices);
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { indices };

        hResult = dx_device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &__indexBuffer);
        assert(SUCCEEDED(hResult));
    }

    // Create Sampler State
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

        HRESULT hResult = dx_device->CreateSamplerState(&samplerDesc, &__samplerState);
        assert(SUCCEEDED(hResult));
    }


    // Load Image
    int texWidth, texHeight, texNumChannels;
    int texForceNumChannels = 4;
    unsigned char* testTextureBytes = stbi_load("D:\\Projects\\VS\\DirectXModdy\\injected_resources\\test.png", &texWidth, &texHeight,
        &texNumChannels, texForceNumChannels);
    assert(testTextureBytes);
    int texBytesPerRow = 4 * texWidth;

    // Create Texture
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = texWidth;
        textureDesc.Height = texHeight;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = testTextureBytes;
        textureSubresourceData.SysMemPitch = texBytesPerRow;

        ID3D11Texture2D* texture;
        HRESULT hResult = dx_device->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);
        assert(SUCCEEDED(hResult));

        hResult = dx_device->CreateShaderResourceView(texture, nullptr, &__textureView);
        assert(SUCCEEDED(hResult));
        texture->Release();
    }

    // constants buffer
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth = sizeof(__Constants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = dx_device->CreateBuffer(&constantBufferDesc, nullptr, &__constantBuffer);
        assert(SUCCEEDED(hResult));
    }

    // bonus stuff for testing //
    // rasterizer state
    {
        D3D11_RASTERIZER_DESC soliddesc;
        ZeroMemory(&soliddesc, sizeof(D3D11_RASTERIZER_DESC));
        soliddesc.FillMode = D3D11_FILL_SOLID;
        soliddesc.CullMode = D3D11_CULL_NONE;

        HRESULT hResult = dx_device->CreateRasterizerState(&soliddesc, &SolidRasterState);
        assert(SUCCEEDED(hResult));
    }

    // depth buffer
    {
        D3D11_DEPTH_STENCIL_DESC depthDesc;
        ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
        depthDesc.DepthEnable = true;
        depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

        HRESULT hResult = dx_device->CreateDepthStencilState(&depthDesc, &SolidDepthStencilState);
        assert(SUCCEEDED(hResult));
    }

    // even more bonus stuffers //
    // depth stencil
    {
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        D3D11_TEXTURE2D_DESC dsDesc;
        dsDesc.Width = clientRect.right - clientRect.left;
        dsDesc.Height = clientRect.bottom - clientRect.top;
        dsDesc.MipLevels = 1;
        dsDesc.ArraySize = 1;
        dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsDesc.SampleDesc.Count = 1;
        dsDesc.SampleDesc.Quality = 0;
        dsDesc.Usage = D3D11_USAGE_DEFAULT;
        dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        dsDesc.CPUAccessFlags = 0;
        dsDesc.MiscFlags = 0;

        HRESULT hResult = dx_device->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
        assert(SUCCEEDED(hResult));
        hResult = dx_device->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);
        assert(SUCCEEDED(hResult));
    }

    ReloadViewportStuff(swap_chain, dx_device, hwnd);
    //cout << "init COMPLETE" << endl;
}

void ObjRender(ID3D11DeviceContext1* dx_device_context) {

    // Calculate view matrix from camera data
    float3 __camera_position = { 0.0f, 0.0f, 2.0f };
    float __camera_yaw = 0;
    float __camera_pitch = 0;
    float4x4 __viewMat = translationMat(-__camera_position) * rotateYMat(-__camera_yaw) * rotateXMat(-__camera_pitch);

    // Spin the cube
    //float4x4 modelMat = scaleMat(float3{ 0.5f, 0.5f, 0.5f }) * translationMat(float3{ 5, 0, 0 });
    float4x4 __modelMat1 = translationMat(float3{ -2,  0,  0 });
    float4x4 __modelMat2 = translationMat(float3{  0, -2,  0 });
    float4x4 __modelMat3 = translationMat(float3{  0,  0, -2 });
    //float4x4 __modelMat1 = translationMat(float3{  2,  0,  0 });
    //float4x4 __modelMat2 = translationMat(float3{  0,  2,  0 });
    //float4x4 __modelMat3 = translationMat(float3{  0,  0,  2 });



    // Calculate model-view-projection matrix to send to shader
    float4x4 __modelViewProj1 = __modelMat1 * __viewMat * __perspectiveMat;
    float4x4 __modelViewProj2 = __modelMat2 * __viewMat * __perspectiveMat;
    float4x4 __modelViewProj3 = __modelMat3 * __viewMat * __perspectiveMat;

    //cout << endl << "matrix: " << endl;
    //cout << __perspectiveMat.m[0][0] << ", " << __perspectiveMat.m[0][1] << ", " << __perspectiveMat.m[0][2] << ", " << __perspectiveMat.m[0][3] << ", " << endl;
    //cout << __perspectiveMat.m[1][0] << ", " << __perspectiveMat.m[1][1] << ", " << __perspectiveMat.m[1][2] << ", " << __perspectiveMat.m[1][3] << ", " << endl;
    //cout << __perspectiveMat.m[2][0] << ", " << __perspectiveMat.m[2][1] << ", " << __perspectiveMat.m[2][2] << ", " << __perspectiveMat.m[2][3] << ", " << endl;
    //cout << __perspectiveMat.m[3][0] << ", " << __perspectiveMat.m[3][1] << ", " << __perspectiveMat.m[3][2] << ", " << __perspectiveMat.m[3][3] << ", " << endl;

    dx_device_context->OMSetRenderTargets(1, &d3d11RenderTargetView, 0);
    dx_device_context->RSSetViewports(1, &viewport);
    dx_device_context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    dx_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx_device_context->IASetInputLayout(__inputLayout);

    dx_device_context->VSSetShader(__vertexShader, nullptr, 0);
    dx_device_context->PSSetShader(__pixelShader, nullptr, 0);

    dx_device_context->VSSetConstantBuffers(0, 1, &__constantBuffer);

    dx_device_context->PSSetShaderResources(0, 1, &__textureView);
    dx_device_context->PSSetSamplers(0, 1, &__samplerState);

    dx_device_context->IASetVertexBuffers(0, 1, &__vertexBuffer, &__stride, &__offset);
    dx_device_context->IASetIndexBuffer(__indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    dx_device_context->RSSetState(SolidRasterState);
    dx_device_context->OMSetDepthStencilState(SolidDepthStencilState, 0);

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    dx_device_context->Map(__constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    __Constants* constants = (__Constants*)(mappedSubresource.pData);
    constants->modelViewProj = __modelViewProj1;
    dx_device_context->Unmap(__constantBuffer, 0);
    dx_device_context->DrawIndexed(__numIndices, 0, 0);
    

    // then make another draw call
    dx_device_context->Map(__constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    constants = (__Constants*)(mappedSubresource.pData);
    constants->modelViewProj = __modelViewProj2;
    dx_device_context->Unmap(__constantBuffer, 0);
    dx_device_context->DrawIndexed(__numIndices, 0, 0);

    // make a 3rd draw call
    dx_device_context->Map(__constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    constants = (__Constants*)(mappedSubresource.pData);
    constants->modelViewProj = __modelViewProj3;
    dx_device_context->Unmap(__constantBuffer, 0);
    dx_device_context->DrawIndexed(__numIndices, 0, 0);


    doodybug_number |= 4;
}