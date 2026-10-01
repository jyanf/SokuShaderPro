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

```cpp
bool __thiscall InitEffectShaders(CBaseEffect *this,void *pdata,uint dsize) {
  if (this->peffect) {
    return false;
  }
  FUN_004153a0(this); 
  return !D3DXCreateEffect(//0x8572a8 c2154800 addr D3DX9_33.DLL::D3DXCreateEffect
      ADDR_D3D9_DEVICE,//0x8a0e30
      pdata,dsize,
      0,0,0,0,
      &this->peffect,0
  );//0x4181c7 e8 e6744000    CALL D3DX9_33.DLL::D3DXCreateEffect
}
```

> fxc /T fx_2_0 /Fo effect.bin effect.fx