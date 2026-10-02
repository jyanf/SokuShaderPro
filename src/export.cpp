#include "main.hpp"

#include "SokuLib.hpp"
#include <Windows.h>
#include <Shlwapi.h>

HMODULE spr::hModule = NULL;
const std::filesystem::path& spr::GetModuleFolder() {
	static std::filesystem::path cache{};
	if (cache.empty()) {
		std::wstring buffer;
		int len = MAX_PATH + 1;
		do {
			buffer.resize(len);
			len = GetModuleFileNameW(hModule, buffer.data(), buffer.size());
		} while (len > buffer.size());
		if (len) cache = std::filesystem::path(buffer.begin(), buffer.end()).parent_path();
	}
	return cache;
}

// We check if the game version is what we target (in our case, Soku 1.10a).
extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return memcmp(hash, SokuLib::targetHash, sizeof(SokuLib::targetHash)) == 0;
}

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
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(h, &mode);
	SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
	spr::hModule = hMyModule;


	VirtualProtect((LPVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);

	spr::Initialize();
	
	VirtualProtect((LPVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
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