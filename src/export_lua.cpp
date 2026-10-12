#ifdef SPR_LUA_EXPORT

#include "main.hpp"
#include <lua.hpp>
#include <LuaBridge/Luabridge.h>
//#include <LuaBridge/Array.h>
//#include <LuaBridge/Vector.h>
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

static bool lua_CheckCompileSuccess(const std::string& name) {
    return CheckCompileSuccess(name.c_str());
}

static int lua_GetShaderIdFromURI(const std::string& uri) {
    return GetShaderIdFromURI(uri.c_str());
}

static int lua_GetEffectReady(lua_State* L) {
    int shaderType = luaL_checkinteger(L, 1);
    void* handle = GetEffectReady(shaderType);
    if (handle) {
        luabridge::Stack<spr::Effect*>::push(L, (spr::Effect*)handle);
    } else return 0;
    return 1;
}

//static int lua_Effect_SetValues(lua_State* L) {//too complex!
static int lua_Effect_SetFloats(lua_State* L) {
    auto fx = luabridge::Stack<spr::Effect*>::get(L, 1).valueOr(nullptr);
    auto name = luaL_checkstring(L, 2);
    if (!fx || !name) {
        return 0;
    }
    if (lua_istable(L, 3)) {
        const auto& vec = luabridge::Stack<std::vector<float>>::get(L, 3);
        if (!vec.value().size()) return 0;
        lua_pushboolean(L, fx->Set(name, vec.value().data(), vec.value().size()));
    } else if (lua_isnumber(L, 3)) {
        float v = lua_tonumber(L, 3);
        lua_pushboolean(L, fx->Set(name, &v));
    } else return 0;
    
    return 1;
}
static int lua_Effect_SetInts(lua_State* L) {
    auto fx = luabridge::Stack<spr::Effect*>::get(L, 1).valueOr(nullptr);
    auto name = luaL_checkstring(L, 2);
    if (!fx || !name) {
        return 0;
    }
    if (lua_istable(L, 3)) {
        const auto& vec = luabridge::Stack<std::vector<INT>>::get(L, 3);
        if (!vec.value().size()) return 0;
        lua_pushboolean(L, fx->Set(name, vec.value().data(), vec.value().size()));
    }
    else if (lua_isnumber(L, 3)) {
        int v = lua_tointeger(L, 3);
        lua_pushboolean(L, fx->Set(name, &v));
    } else return 0;

    return 1;
}
static int lua_Effect_SetBools(lua_State* L) {
    auto fx = luabridge::Stack<spr::Effect*>::get(L, 1).valueOr(nullptr);
    auto name = luaL_checkstring(L, 2);
    if (!fx || !name) {
        return 0;
    }
    if (lua_istable(L, 3)) {
        const size_t count = luaL_len(L, 3);
        //const auto& vec = luabridge::Stack<std::vector<BOOL>>::get(L, 3);
        std::vector<BOOL> vec(count);//this is actually vector<int>
        if (!vec.size()) return 0;
        for (size_t i = 1; i <= count; ++i) {
            lua_rawgeti(L, 3, i);
            if (!lua_isboolean(L, -1)) {
                lua_pop(L, 1);
                return luaL_error(L, "expected boolean table");
            }
            vec[i-1] = lua_toboolean(L, -1);
            lua_pop(L, 1);
        }
        //lua_pushboolean(L, fx->Set(name, vec.data(), vec.size()));
        if (!fx->effect) {
            lua_pushboolean(L, false);
            return 1;
        }
        lua_pushboolean(L, SUCCEEDED(fx->effect->SetBoolArray(name, vec.data(), vec.size())));
    }
    else if (lua_isboolean(L, 3)) {
        bool v = lua_toboolean(L, 3);
        lua_pushboolean(L, fx->Set(name, &v));
    } else return 0;

    return 1;
}
static int lua_Effect_SetTexture(lua_State* L) {
    auto fx = luabridge::Stack<spr::Effect*>::get(L, 1).valueOr(nullptr);
    auto name = luaL_checkstring(L, 2);
    if (!fx || !name) {
        return 0;
    }
    if (lua_isinteger(L, 3)) {
        auto v = reinterpret_cast<ID3DXBaseEffect*>(lua_tointeger(L, 3));
        lua_pushboolean(L, fx->Set(name, v));
    } else return 0;

    return 1;
}
static int lua_Effect_SetString(lua_State* L) {
    auto fx = luabridge::Stack<spr::Effect*>::get(L, 1).valueOr(nullptr);
    auto name = luaL_checkstring(L, 2);
    if (!fx || !name) {
        return 0;
    }
    auto str = luaL_checkstring(L, 3);
    if (str) lua_pushboolean(L, fx->Set(name, str));
    else return 0;

    return 1;
}
static int lua_Effect_SetRaw(lua_State* L) {
    auto fx = luabridge::Stack<spr::Effect*>::get(L, 1).valueOr(nullptr);
    auto name = luaL_checkstring(L, 2);
    if (!fx || !name) {
        return 0;
    }
    std::string str = luaL_checkstring(L, 3);
    if (!str.empty()) lua_pushboolean(L, fx->Set(name, (byte*)str.data(), str.size()));
    else return 0;

    return 1;
}
static int lua_Effect_SetVectors(lua_State* L) {
    auto fx = luabridge::Stack<spr::Effect*>::get(L, 1).valueOr(nullptr);
    auto name = luaL_checkstring(L, 2);
    if (!fx || !name) {
        return 0;
    }
    if (lua_istable(L, 3)) {
        //auto ref = luabridge::LuaRef::fromStack(L, 3);
        if (!luaL_len(L, 3)) return 0;
        lua_rawgeti(L, 3, 1);
        int firstType = lua_type(L, -1);
        lua_pop(L, 1);
        if (firstType==LUA_TNUMBER) {
            const auto& vec = luabridge::Stack<std::array<float, 4>>::get(L, 3);
            if (!vec) return 0;
            lua_pushboolean(L, fx->Set(name, (D3DXVECTOR4*)&vec.value()));
        } else if (firstType==LUA_TTABLE){
            const auto& vec = luabridge::Stack<std::vector<std::array<float, 4>>>::get(L, 3);
            if (!vec) return 0;
            lua_pushboolean(L, fx->Set(name, (D3DXVECTOR4*)vec.value().data(), vec.value().size()));
        }
    } else return 0;

    return 1;
}
static int lua_Effect_SetMatrixes(lua_State* L) {
    auto fx = luabridge::Stack<spr::Effect*>::get(L, 1).valueOr(nullptr);
    auto name = luaL_checkstring(L, 2);
    if (!fx || !name) {
        return 0;
    }
    if (lua_istable(L, 3)) {
        //auto ref = luabridge::LuaRef::fromStack(L, 3);
        if (!luaL_len(L, 3)) return 0;
        lua_rawgeti(L, 3, 1);
        int firstType = lua_type(L, -1);
        lua_pop(L, 1);
        if (firstType==LUA_TNUMBER) {
            const auto& vec = luabridge::Stack<std::array<float, 16>>::get(L, 3);
            if (!vec) return 0;
            lua_pushboolean(L, fx->Set(name, (D3DXMATRIX*)&vec.value()));
        } else if (firstType==LUA_TTABLE) {
            const auto& vec = luabridge::Stack<std::vector<std::array<float, 16>>>::get(L, 3);
            if (!vec) return 0;
            lua_pushboolean(L, fx->Set(name, (D3DXMATRIX*)vec.value().data(), vec.value().size()));
        }
    }
    else return 0;

    return 1;
}

static int lua_SetEffectCallback(lua_State* L) {
    auto fx = spr::EffectManager::instance().get(luaL_checkstring(L, 1));
    if (!fx) return 0;
    if (lua_isuserdata(L, 2)) {//hasty check
        if (luaL_getmetafield(L, 2, "__call") == LUA_TNIL)
            return 0;
        lua_pop(L, 1);
        fx->SetCallbackLua(L);
    } else {
        fx->UnsetCallbackLua();
    }
    return 0;
}
template <typename T> static inline T& castFromPtr(size_t addr) { return *(T*)addr; }//ty enebe

// Module open function for require("spr")
extern "C" __declspec(dllexport) int luaopen_ShaderPro(lua_State* L) {
    using namespace luabridge;
    if (spr::hModule == NULL) {
        const char* msg = "Failed to load ShaderPro: DLL not initialized.";
        luaL_error(L, "%s", msg);
        return 0;
    }
    using spr::Effect;
    getGlobalNamespace(L).beginNamespace("ShaderPro")
        .addFunction("SubmitCompile", &lua_SubmitCompile) // (name, data_or_path, isFile)
        .addFunction("SubmitCompileFromFile", &lua_SubmitCompileFromFile) // (name, utf8Path)
        .addFunction("SubmitCompileFromData", &lua_SubmitCompileFromData) // (name, dataString)
        .addFunction("CheckCompileSuccess", &lua_CheckCompileSuccess) // (name) -> boolean
        .addFunction("GetShaderIdFromURI", &lua_GetShaderIdFromURI) // (uri)
        .addFunction("GetEffectReady", &lua_GetEffectReady) // (shaderType) -> Effect*
        //.addFunction("Effect_Begins", &lua_Effect_Begins)
        //.addFunction("Effect_Ends", &lua_Effect_Ends)
        .beginClass<Effect>("Effect")
            .addStaticFunction("fromPtr", &castFromPtr<Effect>)
            .addFunction("Begins", &Effect::Begins)
            .addFunction("Ends", &Effect::Ends)
            .addFunction("SetFloats", &lua_Effect_SetFloats)
            .addFunction("SetBools", &lua_Effect_SetBools)
            .addFunction("SetInts", &lua_Effect_SetInts)
            .addFunction("SetVectors", &lua_Effect_SetVectors)
            .addFunction("SetMatrixes", &lua_Effect_SetMatrixes)
            .addFunction("SetTexture", &lua_Effect_SetTexture)
            .addFunction("SetString", &lua_Effect_SetString)
            .addFunction("SetRaw", &lua_Effect_SetRaw)
            .addFunction("Commits", &Effect::Commits)
        .endClass()
        .addFunction("SetEffectCallback", lua_SetEffectCallback)// (memory.Callback)
    .endNamespace();
    return 1;
}

#endif // !SPR_LUA_EXPORT