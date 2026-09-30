#include <Shlwapi.h>
#include "SokuLib.hpp"
#include <string>
#include <iostream>

#include "hooks.hpp"

PFN_D3DXCreateEffect oD3DXCreateEffect;
HRESULT WINAPI hkD3DXCreateEffect(
    LPDIRECT3DDEVICE9 device,
    LPCVOID src,
    UINT srcLen,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    LPD3DXEFFECTPOOL pool,
    LPD3DXEFFECT* outEffect,
    LPD3DXBUFFER* errors
) {
    HRESULT hr = oD3DXCreateEffect(
        device, src, srcLen,
        defines, include, flags,
        pool, outEffect, errors);

    if (SUCCEEDED(hr) && outEffect && *outEffect) {
        ID3DXBuffer* disasm = nullptr;
        printf("//Catched binary at %p of size %d bytes!\n//Parsing...\n", src, srcLen);
        HRESULT hr2 = D3DXDisassembleEffect(
            *outEffect,
            FALSE,
            &disasm);

        if (SUCCEEDED(hr2) && disasm) {
            auto data = reinterpret_cast<const char*>(disasm->GetBufferPointer());
            SIZE_T size = disasm->GetBufferSize();

            std::cout << std::endl
                << std::string_view(data, size)
                << std::endl;
            puts("");

            disasm->Release();
        } else {
            puts("//Failed!");
        }
    }

    return hr;
}

constexpr DWORD HookAddr = 0x4181c7;
// Called when the mod loader is ready to initialize this module.
// All hooks should be placed here. It's also a good moment to load settings from the ini.
extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	DWORD old;

#ifdef _DEBUG
	FILE* _;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif
    VirtualProtect((void*)HookAddr, 5, PAGE_READWRITE, &old);

    oD3DXCreateEffect = SokuLib::TamperNearCall(HookAddr, &hkD3DXCreateEffect);
    
    VirtualProtect((void*)HookAddr, 5, old, &old);
	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}

// We check if the game version is what we target (in our case, Soku 1.10a).
extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return memcmp(hash, SokuLib::targetHash, sizeof(SokuLib::targetHash)) == 0;
}

// New mod loader functions
// Loading priority. Mods are loaded in order by ascending level of priority (the highest first).
// When 2 mods define the same loading priority the loading order is undefined.
extern "C" __declspec(dllexport) int getPriority()
{
	return 0;
}

// Not yet implemented in the mod loader, subject to change
// SokuModLoader::IValue **getConfig();
// void freeConfig(SokuModLoader::IValue **v);
// bool commitConfig(SokuModLoader::IValue *);
// const char *getFailureReason();
// bool hasChainedHooks();
// void unHook();