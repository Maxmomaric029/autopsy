#pragma once
#include <d3d11.h>
#include <string>
#include <cstdio>

// ========================================================================
// stb_image-based texture loader
// Loads PNG/JPG/... files into ID3D11ShaderResourceView for ImGui::Image
// ========================================================================

// Forward declarations for stb_image (implemented in graphic.cpp)
extern "C" {
    extern unsigned char* stbi_load_from_memory(const unsigned char* buffer,
        int len, int* x, int* y, int* n, int req_comp);
    extern unsigned char* stbi_load(const char* filename, int* x, int* y,
        int* n, int req_comp);
    extern void stbi_image_free(void*);
}

namespace tex {

    // ========================================================================
    // Helper: create a D3D11 SRV from raw RGBA pixel data
    // ========================================================================
    inline bool create_srv(ID3D11Device* device,
        unsigned char* imageData, int width, int height,
        ID3D11ShaderResourceView** outSRV) {

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = (UINT)width;
        desc.Height = (UINT)height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = imageData;
        initData.SysMemPitch = (UINT)(width * 4);

        ID3D11Texture2D* texture = nullptr;
        HRESULT hr = device->CreateTexture2D(&desc, &initData, &texture);
        if (FAILED(hr) || !texture) {
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(texture, &srvDesc, outSRV);
        texture->Release();

        return SUCCEEDED(hr) && *outSRV;
    }

    // ========================================================================
    // Load a texture from embedded memory (e.g. PNG->C array)
    // Returns true on success. The caller must Release() the SRV when done.
    // ========================================================================
    inline bool load_from_memory(ID3D11Device* device,
        const unsigned char* data, unsigned int dataSize,
        ID3D11ShaderResourceView** outSRV,
        int* outWidth = nullptr, int* outHeight = nullptr) {

        if (!device || !data || !dataSize || !outSRV) return false;
        *outSRV = nullptr;

        int width = 0, height = 0, channels = 0;
        unsigned char* imageData = stbi_load_from_memory(
            data, (int)dataSize, &width, &height, &channels, 4);

        if (!imageData) return false;

        bool ok = create_srv(device, imageData, width, height, outSRV);
        stbi_image_free(imageData);

        if (ok) {
            if (outWidth)  *outWidth  = width;
            if (outHeight) *outHeight = height;
        }
        return ok;
    }

    // ========================================================================
    // Load a texture from a file into a D3D11 shader resource view
    // Returns true on success. The caller must Release() the SRV when done.
    // ========================================================================
    inline bool load(ID3D11Device* device, const char* filePath,
        ID3D11ShaderResourceView** outSRV, int* outWidth = nullptr,
        int* outHeight = nullptr) {

        if (!device || !filePath || !outSRV) return false;
        *outSRV = nullptr;

        int width = 0, height = 0, channels = 0;
        unsigned char* imageData = stbi_load(filePath, &width, &height,
            &channels, 4); // Force RGBA

        if (!imageData) return false;

        bool ok = create_srv(device, imageData, width, height, outSRV);
        stbi_image_free(imageData);

        if (ok) {
            if (outWidth)  *outWidth  = width;
            if (outHeight) *outHeight = height;
        }
        return ok;
    }

} // namespace tex
