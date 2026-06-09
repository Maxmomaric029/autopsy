#pragma once
#include <d3d11.h>
#include <string>
#include <cstdio>

// ========================================================================
// stb_image-based texture loader
// Loads PNG/JPG/... files into ID3D11ShaderResourceView for ImGui::Image
// ========================================================================

namespace tex {

    // ========================================================================
    // Load a texture from a file into a D3D11 shader resource view
    // Returns true on success. The caller must Release() the SRV when done.
    // ========================================================================
    inline bool load(ID3D11Device* device, const char* filePath,
        ID3D11ShaderResourceView** outSRV, int* outWidth = nullptr,
        int* outHeight = nullptr) {

        if (!device || !filePath || !outSRV) return false;
        *outSRV = nullptr;

        // stb_image implementation is in graphic.cpp where STB_IMAGE_IMPLEMENTATION
        // is defined. This header only uses it as a forward-declared function.
        // We define the stb_image functions as extern here since they're
        // implemented in graphic.cpp via the #define STB_IMAGE_IMPLEMENTATION.

        // Load via stb_image
        int width = 0, height = 0, channels = 0;

        // Forward declaration - implemented in graphic.cpp
        extern unsigned char* stbi_load(const char* filename, int* x, int* y,
            int* n, int req_comp);

        unsigned char* imageData = stbi_load(filePath, &width, &height,
            &channels, 4); // Force RGBA

        if (!imageData) return false;

        // Create D3D11 texture
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
            extern void stbi_image_free(void*);
            stbi_image_free(imageData);
            return false;
        }

        // Create shader resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(texture, &srvDesc, outSRV);
        texture->Release();

        extern void stbi_image_free(void*);
        stbi_image_free(imageData);

        if (FAILED(hr) || !*outSRV) return false;

        if (outWidth)  *outWidth  = width;
        if (outHeight) *outHeight = height;

        return true;
    }

} // namespace tex
