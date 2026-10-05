#ifdef SPR_LUA_EXPORT

#include "main.hpp"
extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}
#include <Luabridge/Luabridge.h>
#include "export.hpp"
#include <string>
#include <vector>
#include <Windows.h>
namespace {
    // Helper: UTF-8 -> wide string
    static std::wstring utf8_to_wstring(const std::string& s) {
        if (s.empty()) return {};
        int wlen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        if (wlen == 0) return {};
        std::wstring out;
        out.resize(wlen);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], wlen);
        return out;
    }
}
// Lua wrappers that call exported C functions from export.hpp

static void lua_SubmitCompile(const std::string& name, const std::string& data, const std::string& path) {
    std::string pathUtf8 = path;
    std::wstring wpath = utf8_to_wstring(pathUtf8);
    return SubmitCompile(name.c_str(), data.data(), data.size(), wpath.c_str());
}

static void lua_SubmitCompileFromFile(const std::string& name, const std::string& pathUtf8) {
    std::wstring wpath = utf8_to_wstring(pathUtf8);
    return SubmitCompileFromFile(name.c_str(), wpath.c_str());
}

static void lua_SubmitCompileFromData(const std::string& name, const std::string& data) {
    return SubmitCompileFromData(name.c_str(), data.data(), data.size());
}

static int lua_GetShaderIdFromURI(const std::string& uri) {
    return GetShaderIdFromURI(uri.c_str());
}

static void* lua_GetEffectReady(int shaderType) {
    return GetEffectReady(shaderType);
}

static bool lua_Effect_Begins(void* effectPtr) {
    return Effect_Begins(effectPtr);
}
static bool lua_Effect_Ends(void* effectPtr) {
    return Effect_Ends(effectPtr);
}

// Optional: helper to get Effect by name (via Submit/lookup) - attempt to get pointer via shader id 0 if needed.
// We expose GetEffectByName that returns the opaque pointer by checking EffectManager (via exported API GetEffectReady not available by name).
// If you need by-name lookup, add a C export that returns pointer by name.
static void lua_Noop() { /* placeholder if needed */ }

// Module open function for require("spr")
extern "C" __declspec(dllexport) int luaopen_ShaderPro(lua_State* L) {
    using namespace luabridge;
    if (spr::hModule == NULL) {
        const char* msg = "Failed to load ShaderPro: DLL not initialized.";
        luaL_error(L, "%s", msg);
        return 0;
    }
    getGlobalNamespace(L).beginNamespace("ShaderPro")
        .addFunction("SubmitCompile", &lua_SubmitCompile) // (name, data_or_path, isFile)
        .addFunction("SubmitCompileFromFile", &lua_SubmitCompileFromFile) // (name, utf8Path)
        .addFunction("SubmitCompileFromData", &lua_SubmitCompileFromData) // (name, dataString)
        .addFunction("GetShaderIdFromURI", &lua_GetShaderIdFromURI) // (uri)
        .addFunction("GetEffectReady", &lua_GetEffectReady) // (shaderType) -> lightuserdata
        .addFunction("Effect_Begins", &lua_Effect_Begins)
        .addFunction("Effect_Ends", &lua_Effect_Ends)
    .endNamespace();
    return 1;
}

#endif // !SPR_LUA_EXPORT