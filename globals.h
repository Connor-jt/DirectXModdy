#pragma once


class DLLGLobals {
public:
    void* last_d3d11DeviceContext;
    void* last_ID3D11Buffer; // from VSSetConstantBuffers
    void* last_ID3D11VertexShader; // from VSSetShader

    int unique_swap_chains = 0;
    void* actual_d3d11DeviceContext;
};