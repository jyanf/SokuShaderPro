https://learn.microsoft.com/zh-cn/windows/win32/direct3d9/d3dxdisassembleeffect

```cpp
typedef HRESULT (WINAPI *PFN_D3DXCreateEffect)(
    LPDIRECT3DDEVICE9 pDevice,
    LPCVOID pSrcData,
    UINT SrcDataLen,
    CONST D3DXMACRO *pDefines,
    LPD3DXINCLUDE pInclude,
    DWORD Flags,
    LPD3DXEFFECTPOOL pPool,
    LPD3DXEFFECT *ppEffect,
    LPD3DXBUFFER *ppCompilationErrors
);

HRESULT WINAPI hkD3DXCreateEffect(
    LPDIRECT3DDEVICE9 device,
    LPCVOID src,
    UINT srcLen,
    const D3DXMACRO *defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    LPD3DXEFFECTPOOL pool,
    LPD3DXEFFECT *outEffect,
    LPD3DXBUFFER *errors
) {
    HRESULT hr = oD3DXCreateEffect(
        device, src, srcLen,
        defines, include, flags,
        pool, outEffect, errors);

    if (SUCCEEDED(hr) && outEffect && *outEffect) {
        ID3DXBuffer *disasm = nullptr;

        HRESULT hr2 = D3DXDisassembleEffect(
            *outEffect,
            FALSE,
            &disasm);

        if (SUCCEEDED(hr2) && disasm) {
            const void *data = disasm->GetBufferPointer();
            SIZE_T size = disasm->GetBufferSize();

            // 保存/解析 data[0..size)

            disasm->Release();
        }
    }

    return hr;
}
```