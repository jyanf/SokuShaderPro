#include <windows.h>
#include <Shlwapi.h>
#include "d3d9.h"
#include "d3dx9.h"
#include "d3dx9effect.h"

typedef HRESULT(WINAPI* PFN_D3DXCreateEffect)(
    LPDIRECT3DDEVICE9 pDevice,
    LPCVOID pSrcData,
    UINT SrcDataLen,
    CONST D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude,
    DWORD Flags,
    LPD3DXEFFECTPOOL pPool,
    LPD3DXEFFECT* ppEffect,
    LPD3DXBUFFER* ppCompilationErrors
    );

extern PFN_D3DXCreateEffect oD3DXCreateEffect;
