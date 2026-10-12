#pragma once
#include <Windows.h>
#include <Shlwapi.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>
#include <optional>
#include <shared_mutex>

#include "d3d9.h"
#include "d3dx9.h"
#include "d3dx9effect.h"

#include "SokuLib.hpp"
#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#ifndef SPR_EXPORTS
#define SPR_EXPORTS
#endif

namespace spr {
	constexpr DWORD SHADER0_BYTECODE_OFFSET = 0x85a608;
	constexpr size_t SHADER0_BYTECODE_SIZE = 0x4d0;
	constexpr DWORD SHADER1_BYTECODE_OFFSET = 0x85f908;
	constexpr size_t SHADER1_BYTECODE_SIZE = 0x1b20;
	constexpr DWORD SHADER2_BYTECODE_OFFSET = 0x861428;
	constexpr size_t SHADER2_BYTECODE_SIZE = 0x684;

	constexpr DWORD EFFECT_BEGIN_FLAG = 0;//D3DXFX_DONOTSAVESAMPLERSTATE;

	struct CBaseEffect {
		constexpr static auto ADDR_C_BASE_EFFECT_VTABLE = (void**)0x871334;
		ID3DXEffect* effect = nullptr;
		inline virtual HRESULT OnLostDevice() {
			//if (effect) effect->OnLostDevice();
			return (this->*SokuLib::union_cast<HRESULT(CBaseEffect::*)()>(ADDR_C_BASE_EFFECT_VTABLE[0]))();
		}
		inline virtual HRESULT OnResetDevice() {
			//if (effect) effect->OnResetDevice();
			return (this->*SokuLib::union_cast<HRESULT(CBaseEffect::*)()>(ADDR_C_BASE_EFFECT_VTABLE[1]))();
		}
		inline ~CBaseEffect() {//non virtual, without unregister listener
			//if (effect) { effect->Release(); effect = nullptr; }
			reinterpret_cast<void(__fastcall*)(CBaseEffect*)>(0x418160)(this);
		}
	};
	class Effect : public CBaseEffect {
		using Base = CBaseEffect;
		static volatile bool _begined;//lock? we don't need that for single render thread
		class RenderGuard {
			Effect* data;
			RenderGuard(const RenderGuard&) = delete;
			RenderGuard& operator=(const RenderGuard&) = delete; RenderGuard& operator=(RenderGuard&&) = delete;
		public:
			inline RenderGuard(RenderGuard&& other) noexcept {
				data = std::exchange(other.data, nullptr);
			}
			inline RenderGuard(Effect& d) : data(&d) {
				data->Begins();
			}
			inline ~RenderGuard() {
				if (data) data->Ends();
			}
		};
		static void __fastcall DefaultCallback(SokuLib::v2::AnimationObject* This, HANDLE This2, const char* tech, const char* pass) {
			SokuLib::DrawUtils::DxSokuColor sc = This->renderInfos.shaderColor;
			auto fx = static_cast<Effect*>(This2);
			D3DXVECTOR4 scf{
				sc.r / 255.0f, sc.g / 255.0f, sc.b / 255.0f, sc.a / 255.0f
			};
			if (fx->Set("ShaderColor", &scf)) {
				fx->Commits();
			}
		}
	public:
		//do not add non-trivial member
		bool enabled = false;
		int tPass=0;
		D3DXHANDLE tTech = NULL;
		void(__fastcall* callback)(SokuLib::v2::AnimationObject*, HANDLE, const char*, const char*) = DefaultCallback;
		lua_State* subL = nullptr; int subroutine = LUA_REFNIL;//usually only one render thread, so no mutex?
		int callbacklua = LUA_REFNIL;
		inline void UnsetCallbackLua() {
			if (subL && callbacklua!=LUA_REFNIL) {
				luaL_unref(subL, LUA_REGISTRYINDEX, callbacklua);
			}
			callbacklua = LUA_REFNIL;
		}
		inline void SetCallbackC(void* pf) {
			UnsetCallbackLua();
			if (pf) {
				callback = reinterpret_cast<decltype(callback)>(pf);
			} else {
				callback = DefaultCallback;
			}
		}
		inline void SetCallbackLua(lua_State* L) {//on stack 1.name, 2.cb
			callback = DefaultCallback;
			if (!subL) {
				subL = lua_newthread(L);
				subroutine = luaL_ref(L, LUA_REGISTRYINDEX);
			}
			lua_pushvalue(L, 2);
			callbacklua = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		inline void __fastcall RunCallback(SokuLib::v2::AnimationObject* This, Effect* fx, const char* tech, const char* pass) {
			bool skip = false;
			if (subL && callbacklua != LUA_REFNIL) {
				//memory.Callback, has lock itself
				lua_rawgeti(subL, LUA_REGISTRYINDEX, callbacklua);
				//luabridge::Stack<Effect*>::push(subL, fx);
				lua_pushinteger(subL, (DWORD)fx);
				lua_pushinteger(subL, (DWORD)This);
				lua_pushstring(subL, tech);
				lua_pushstring(subL, pass);
				if (lua_pcall(subL, 4, 1, 0)) {
					std::cout << lua_tostring(subL, -1);
				} else {
					skip = lua_toboolean(subL, -1);
				} lua_pop(subL, 1);
			}
			if (!skip && callback) {
				callback(This, (HANDLE)fx, tech, pass);
			}
		}

		template<auto FX> bool MyCreateEffect(void* pdata, size_t psize);
		template<auto FX>
		struct OrgCreateEffect {
			using PF = decltype(&Effect::MyCreateEffect<FX>);
			const PF value;
			inline static PF get_or_set(PF in) {
				static PF value = nullptr;
				if (in) {
					return value = in;
				} else return value;
			}
			inline OrgCreateEffect(PF org = nullptr) : value(get_or_set(org)) {}
			inline operator PF() { return value; }
		};
		inline static void AddListenerWrapper(void* obj) {//EAX fix
			constexpr DWORD g_dxContext = 0x8a0e10;
			constexpr uintptr_t addr = 0x4153A0;
			__asm {
				mov eax, g_dxContext
				push obj
				call addr
			}
		}
		inline static void RemoveListenerWrapper(void* obj) {
			return reinterpret_cast<void(__fastcall*)(void*)>(0x415440)(obj);
		}
		static HRESULT CreateEffectWarm(Effect& result, const char* fxname, void* pdata, size_t psize, const std::filesystem::path& filepath={});
		
		void debug(HRESULT result);
		inline Effect(const std::string_view& name, void* embedded_data = nullptr, size_t embedded_size = 0, const std::filesystem::path& filepath = {}) {
			AddListenerWrapper(this);
			auto ret = CreateEffectWarm(*this, name.data(), embedded_data, embedded_size, filepath);
			enabled = ret == D3D_OK;
			debug(ret);
		}
		inline ~Effect() {//with unregister
			if (subL && subroutine) {
				luaL_unref(subL, LUA_REGISTRYINDEX, subroutine);
			}
			//RemoveListenerWrapper(this); CBaseEffect::~CBaseEffect();
			reinterpret_cast<void(__fastcall*)(CBaseEffect*)>(0x4181e0)(this);
		}
		inline bool check() {
			return this->effect && enabled;
		}
		template<typename T> bool Set(LPCSTR key, const T* data, UINT count=1) {
			if (!effect || !data)
				return false;
			HRESULT ret = -1;
			if constexpr (std::is_same_v<T, bool>) {
				if (count <= 1) {
					BOOL temp = *data;
					ret = effect->SetBool(key, *static_cast<const BOOL*>(&temp));
				}
				else {
					std::vector<BOOL> temp(count);
					for (int i = 0; i < count; ++i) {
						temp[i] = data[i];
					}
					ret = effect->SetBoolArray(key, static_cast<const BOOL*>(temp.data()), count);
				}
			}
			else if constexpr (std::is_same_v<T, INT>) {
				if (count <= 1)
					ret = effect->SetInt(key, *static_cast<const INT*>(data));
				else
					ret = effect->SetIntArray(key, static_cast<const INT*>(data), count);
			}
			else if constexpr (std::is_same_v<T, FLOAT>) {
				if (count <= 1)
					ret = effect->SetFloat(key, *static_cast<const FLOAT*>(data));
				else
					ret = effect->SetFloatArray(key, static_cast<const FLOAT*>(data), count);
			}
			else if constexpr (std::is_same_v<T, D3DXVECTOR4>) {
				if (count <= 1)
					ret = effect->SetVector(key, static_cast<const D3DXVECTOR4*>(data));
				else
					ret = effect->SetVectorArray(key, static_cast<const D3DXVECTOR4*>(data), count);
			}
			else if constexpr (std::is_same_v<T, D3DXMATRIX>) {
				if (count <= 1)
					ret = effect->SetMatrix(key, static_cast<const D3DXMATRIX*>(data));
				else
					ret = effect->SetMatrixArray(key, static_cast<const D3DXMATRIX*>(data), count);
			}
			else if constexpr (std::derived_from<T, IDirect3DBaseTexture9>) {
				ret = effect->SetTexture(key, static_cast<IDirect3DBaseTexture9*>(const_cast<T*>(data)));
			}
			else if constexpr (std::is_same_v<T, CHAR>) {
				ret = effect->SetString(key, static_cast<LPCSTR>(data));
			}
			else {
				ret = effect->SetValue(key, data, sizeof(T) * count);
			}
			return SUCCEEDED(ret);
		}
		inline bool Commits() {
			if (check()) {
				return SUCCEEDED(this->effect->CommitChanges());
			} 
			return false; 
		}
		inline bool Switch(LPCSTR techName, short pass=-1) {
			if (!check() || _begined) return false;
			if (techName && !(this->tTech = this->effect->GetTechniqueByName(techName))) return false;
			this->tPass = pass>=0 ? pass : this->tPass;
			return true;
		}//you kidding me, cannot get pass index by name??
		inline bool Switch(int tech, short pass = -1) {
			if (!check() || _begined) return false;
			if (tech>=0 && !(this->tTech = this->effect->GetTechnique(tech))) return false;
			this->tPass = pass >= 0 ? pass : this->tPass;
			return true;
		}

		inline bool Begins(int passOverride=-1) {
			if (!check() || _begined) return false;
			if (tTech) {
				this->effect->SetTechnique(tTech);
				tTech = NULL;
			}
			this->effect->Begin(nullptr, EFFECT_BEGIN_FLAG);//auto saves states
			this->effect->BeginPass(passOverride>=0 ? passOverride : this->tPass);
			_begined = true;
			return true;
		}
		inline bool Ends() {
			if (!check() || !_begined) return false;
			this->effect->EndPass();
			this->effect->End();//auto recover states
			_begined = false;
			return true;
		}
		inline RenderGuard GetRenderGuard() { return *this; }
	};
	class EffectManager : protected std::unordered_map<std::string, Effect*> {
		using Key = value_type::first_type;
		using Value = value_type::second_type;
		using Base = std::unordered_map<Key, Value>;
		using Path = std::filesystem::path;
		//singleton
		EffectManager() = default;
		EffectManager(const EffectManager&) = delete; EffectManager(EffectManager&&) = delete;
		EffectManager& operator=(const EffectManager&) = delete; EffectManager& operator=(EffectManager&&) = delete;
		class Tasker {
			friend EffectManager;
			struct Task {
				std::string name;
				Path filepath{};
				void* data = nullptr;// owned copy of data
				size_t size = 0;
				Task() = default;
				Task(const Key& n, const Path& fp, void* d, size_t s) : name(n), filepath(fp), data(d), size(s) {}
			};
			std::queue<Task> waiting;
			// worker thread primitives
			std::thread _worker;
			std::mutex _queueMtx;
			std::condition_variable _queueCv;
			std::atomic<bool> _workerRunning{ false };
			std::atomic<bool> _stopWorker{ false };
		public:
			void workerLoop();
			void startWorkerIfNeeded(int delayms = 0);
			void stopWorker();
		} tasker;
		// LUT tech & pass meta <--> shaderType
#ifndef RESERVE_SHADER_COUNT
#define RESERVE_SHADER_COUNT (256)
#endif
		class LUT {
			struct Entry {
				std::string effectName;
				int techIndex = -1;
				std::string techName;
				int passIndex = -1;
				std::string passName;
			};
			std::mutex _mtx;
			int _nextId = RESERVE_SHADER_COUNT;
			std::unordered_map<int, Entry> _idToEntry;
			std::unordered_map<std::string, std::vector<int>> _effectToIds;
			std::unordered_map<std::string, int> _keyToId;
			inline int _allocId() {
				return _nextId++; // just linear
			}
			void refreshURI(const Entry& entry, int id = 0);
		public:
			LUT() = default;
			LUT(int reserved_no) : _nextId(reserved_no) {}
			void registerEffect(const std::string& effectName, Effect* eff);
			std::optional<int> getShaderTypeByURI(const std::string_view& uri);
			const Entry* getEntryByType(int shaderType);
		} lut;
		mutable std::shared_mutex mtx_; //map_
	public:
		static CBaseEffect& g_EffectBattle;
		//singleton
		static EffectManager& instance() {
			static EffectManager instance;
			return instance;
		}
		inline static void OnClose() {
			// stop worker first
			instance().tasker.stopWorker();
			{ std::unique_lock lock(instance().mtx_);
				for (auto& p : instance()) {
					delete p.second;
				}
				instance().clear();
			}
			if (ogOnClose) return ogOnClose();
		}
		static decltype(&OnClose) ogOnClose;

		inline Value get(const Key& key) {
			std::shared_lock lock(mtx_);
			auto it = this->find(key);
			if (it != this->end()) return it->second;
			return nullptr;
		}
		inline Value& get_or_open(const Key& key, void* ed = nullptr, size_t es = 0, const Path& fp = {}) {
            Effect* created = nullptr;
			std::unique_lock lock(mtx_);
			auto [it, inserted] = this->try_emplace(key, nullptr);
			if (inserted) {
				it->second = new Effect(key, ed, es, fp);
				created = it->second;
			}
			Value& result = it->second;
			lock.unlock();
			if (created) {
				// register in LUT outside of EffectManager lock to avoid lock ordering deadlocks
				lut.registerEffect(key, created);
			}
			return result;
		}
		inline Value& operator[](const Key& key) {
			return get_or_open(key);
		}
      inline void replace(const Key& key, const Value& effect) {
			Value toRegister = nullptr;
			{
				std::unique_lock lock(mtx_);
				auto it = this->find(key);
				if (it != this->end()) {
					delete it->second;
					it->second = effect;
					toRegister = it->second;
				} else {
					(*this)[key] = effect;
					toRegister = (*this)[key];
				}
			}
			// update LUT outside of EffectManager lock to avoid lock ordering deadlocks
			if (toRegister) lut.registerEffect(key, toRegister);
		}

		void AsyncRequire(const Key& key, const void* ed = nullptr, size_t es = 0, const Path& fp = {});

		inline void NotifyTasker(int delay = 0) {
			tasker.startWorkerIfNeeded(delay);
			tasker._queueCv.notify_one();
		}
		inline Effect* LutSwitchShader(int type, std::string_view* names = nullptr) {
			auto entry = lut.getEntryByType(type);
			if (!entry) return nullptr;
			if (names) {
				names[0] = entry->effectName;
				names[1] = entry->techName;
				names[2] = entry->passName;
			}
			Effect* fx = get(entry->effectName);
			if (fx && fx->Switch(entry->techIndex, entry->passIndex)) return fx;
			return nullptr;
		}
		inline std::optional<int> LutFindShader(const std::string& uri) {
			return lut.getShaderTypeByURI(uri);
		}

		inline virtual ~EffectManager() {
			std::unique_lock lock(mtx_);
			tasker.stopWorker();
			for (auto& [k, v] : (*this)) {
				delete (CBaseEffect*)v;//should not remove listener cuz the whole game is closing
			}
		}
	};

	

//inline void logs(const std::string_view& str) {
//#ifdef _DEBUG
//	std::cout << str << std::endl;
//#endif // _DEBUG
//}



void Initialize();

const std::filesystem::path& GetModuleFolder();
std::filesystem::path GetShaderFolder();
std::filesystem::path GetIniPath();



	extern decltype(&ID3DXEffect::End) ogEffectEnd;
	extern CRITICAL_SECTION& g_D3DContextLock;
	extern HMODULE hModule;
}
