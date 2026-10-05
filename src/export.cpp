#include "main.hpp"

#include "SokuLib.hpp"
#include <Windows.h>
#include <Shlwapi.h>

#include "export.hpp"

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
	return 20;
}

// Not yet implemented in the mod loader, subject to change
// SokuModLoader::IValue **getConfig();
// void freeConfig(SokuModLoader::IValue **v);
// bool commitConfig(SokuModLoader::IValue *);
// const char *getFailureReason();
// bool hasChainedHooks();
// void unHook();

//better call it on soku setup
// source or bin?
//SubmitCustomFx[FromFile/String](name, data)

//getType

// Preparation, call them as early as possible!
extern "C" __declspec(dllexport) bool SubmitCompile(const char* name, const void* data, size_t size, const wchar_t* filepath)
{
	if (!name) return false;
	auto& EM = spr::EffectManager::instance();
	EM.AsyncRequire(std::string(name), data, size, filepath);
}

extern "C" __declspec(dllexport) bool SubmitCompileFromFile(const char* name, const wchar_t* filepath)
{
	if (!name || !filepath) return false;
	auto& EM = spr::EffectManager::instance();
	EM.AsyncRequire(std::string(name), nullptr, 0, filepath);
}

extern "C" __declspec(dllexport) bool SubmitCompileFromData(const char* name, const void* data, size_t size)
{
	if (!name || !data || size == 0) return false;
	auto& EM = spr::EffectManager::instance();
	EM.AsyncRequire(std::string(name), data, size);
	return true;
}


// Query shaderType by uri (returns -1 if not found)
extern "C" __declspec(dllexport) int GetShaderIdFromURI(const char* uri)
{
	if (!uri) return 0;
	auto& EM = spr::EffectManager::instance();
	return EM.LutFindShader(std::string(uri)).value_or(0);
}

// Return Effect* as opaque pointer (void*).
// The inner state is set already for begin and end pair
// Caller must not delete.
extern "C" __declspec(dllexport) HANDLE GetEffectReady(int shaderType)
{
	auto& EM = spr::EffectManager::instance();
	spr::Effect* fx = EM.LutSwitchShader(shaderType);
	return reinterpret_cast<HANDLE>(fx);
}

//// Flexible wrappers to call Effect methods
//extern "C" __declspec(dllexport) bool Effect_Switch(void* effectPtr, const char* techName, int pass)
//{
//	if (!effectPtr) return false;
//	auto fx = reinterpret_cast<spr::Effect*>(effectPtr);
//	// techName may be null -> use index-based switch if pass provided
//	if (techName) {
//		return fx->Switch(techName, pass);
//	}
//	else {
//		return fx->Switch(-1, pass); // treat pass as tech index if techName null
//	}
//}

extern "C" __declspec(dllexport) bool Effect_Begins(HANDLE effectPtr)
{
	if (!effectPtr) return false;
	auto fx = reinterpret_cast<spr::Effect*>(effectPtr);
	return fx->Begins();
}

extern "C" __declspec(dllexport) bool Effect_Ends(HANDLE effectPtr)
{
	if (!effectPtr) return false;
	auto fx = reinterpret_cast<spr::Effect*>(effectPtr);
	return fx->Ends();
}
