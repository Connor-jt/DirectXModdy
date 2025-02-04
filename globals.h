#pragma once


class DLLGLobals {
public:
    void* last_d3d11DeviceContext;
    void* last_ID3D11Buffer; // from VSSetConstantBuffers
    void* last_ID3D11VertexShader; // from VSSetShader

    unsigned long long debug1; // func 1 rcx
    unsigned long long debug2; // func 1 incrementor
    unsigned long long debug3; // func 3 access count
    unsigned long long debug4; // func 4 access count
};