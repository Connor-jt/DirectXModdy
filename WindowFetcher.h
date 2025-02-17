#pragma once
ID3D11ShaderResourceView* CreateTextureFromBitmap(BITMAP bmp, ID3D11Device* device, ID3D11DeviceContext* context, std::vector<BYTE> pixels) {
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = bmp.bmWidth;
    desc.Height = bmp.bmHeight;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;



    ID3D11Texture2D* pTexture = nullptr;
    device->CreateTexture2D(&desc, NULL, &pTexture);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    context->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    memcpy(mappedResource.pData, &pixels[0], bmp.bmWidthBytes * bmp.bmHeight);

    context->Unmap(pTexture, 0);

    ID3D11ShaderResourceView* pSRV = nullptr;
    device->CreateShaderResourceView(pTexture, NULL, &pSRV);
    pTexture->Release();

    return pSRV;
}
ID3D11ShaderResourceView* GetTargetProcessScreen(ID3D11DeviceContext* context, ID3D11Device* dx_device, char* window_name) {
    if (!window_name[0]) return 0;

    HWND hwnd = FindWindowA(NULL, window_name);
    if (!hwnd) return 0;

    HDC hdcWindow = GetDC(hwnd);
    if (!hdcWindow) return 0;

    RECT rc;
    GetClientRect(hwnd, &rc);
    float width = rc.right - rc.left;
    float height = rc.bottom - rc.top;

    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);
    //auto var = CreateDIBSection(hdcWindow);
    SelectObject(hdcMemDC, hbmScreen);
    BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);

    BITMAP bmpScreen;
    GetObjectW(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  // Negative to indicate top-down bitmap
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    //int rowSize = ((24 * width + 31) / 32) * 4;
    //std::vector<BYTE> pixels(rowSize * height);
    int total_bytes = bmpScreen.bmWidthBytes * bmpScreen.bmHeight;
    std::vector<BYTE> pixels(total_bytes);
    GetDIBits(hdcWindow, hbmScreen, 0, height, pixels.data(), reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    // reset transparency bits
    for (int i = 3; i < total_bytes; i += 4) {
        pixels[i] = 0xff;
        //char p = pixels[i - 1];
        //pixels[i - 1] = pixels[i - 3];
        //pixels[i - 3] = p;
    }


    auto texture = CreateTextureFromBitmap(bmpScreen, dx_device, context, pixels);
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);

    return texture;
}