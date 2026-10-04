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

#include "d3d9.h"
#include "d3dx9.h"
#include "d3dx9effect.h"

#include "SokuLib.hpp"

namespace spr {
	constexpr DWORD SHADER0_BYTECODE_OFFSET = 0x85a608;
	constexpr size_t SHADER0_BYTECODE_SIZE = 0x4d0;
	constexpr DWORD SHADER1_BYTECODE_OFFSET = 0x85f908;
	constexpr size_t SHADER1_BYTECODE_SIZE = 0x1b20;
	constexpr DWORD SHADER2_BYTECODE_OFFSET = 0x861428;
	constexpr size_t SHADER2_BYTECODE_SIZE = 0x684;

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
	public:
		//do not add non-trivial member
		bool enabled = false;
		int tPass=0;
		D3DXHANDLE tTech = NULL;

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
			//RemoveListenerWrapper(this); CBaseEffect::~CBaseEffect();
			reinterpret_cast<void(__fastcall*)(CBaseEffect*)>(0x4181e0)(this);
		}
		inline bool check() {
			return this->effect && enabled;
		}
		template<typename T> void Set(LPCSTR key, T in);
		inline void Commit() { if (check()) { this->effect->CommitChanges(); } }
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
			this->effect->Begin(nullptr, 0);//auto saves states
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
	class EffectManager : std::unordered_map<std::string, Effect*> {
		using Key = value_type::first_type;
		using Value = value_type::second_type;
		using Base = std::unordered_map<Key, Value>;
		using Path = std::filesystem::path;
		//singleton
		EffectManager() = default;
		EffectManager(const EffectManager&) = delete; EffectManager(EffectManager&&) = delete;
		EffectManager& operator=(const EffectManager&) = delete; EffectManager& operator=(EffectManager&&) = delete;
		inline virtual ~EffectManager() {
			tasker.stopWorker();
			for (auto& [k, v] : (*this)) {
				delete (CBaseEffect*)v;//should not remove listener cuz the whole game is closing
			}
		}
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
			void startWorkerIfNeeded();
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
		public:
			LUT() = default;
			LUT(int reserved_no) : _nextId(reserved_no) {}
			void registerEffect(const std::string& effectName, Effect* eff);
			std::optional<int> getShaderTypeByURI(const std::string_view& uri);
			const Entry* getEntryByType(int shaderType);
		} lut;

	public:
		static CBaseEffect& g_EffectBattle;
		//singleton
		static EffectManager& instance() {
			static EffectManager instance;
			return instance;
		}
		inline static void OnClose() {
			instance().tasker.stopWorker();
			for (auto& [k, v] : instance()) {
				delete v;
			}
			instance().clear();
			if (ogOnClose) return ogOnClose();
		}
		static decltype(&OnClose) ogOnClose;

		inline Value get(Key key) {
			auto it = this->find(key);
			if (it!=this->end()) {
				return it->second;
			}
			return nullptr;
		}
		inline Value& get_or_open(Key key, void* ed=nullptr, size_t es=0, const Path& fp={}) {
			auto [it, inserted] = this->try_emplace(key, nullptr);
			if (inserted) {
				it->second = new Effect(key, ed, es, fp);
				lut.registerEffect(key, it->second);
			}
			return it->second;
		}
		inline Value& operator[](Key key) {
			return get_or_open(key);
		}
		inline void close(Key key) {
			auto it = this->find(key);
			if (it != this->end()) {
				delete it->second;
				this->erase(it);
			}
		}

		/*
		void require(Key key, void* ed = nullptr, size_t es = 0) {
			waiting.emplace(key, ed, es);
		}
		void open_all() {
			while (!waiting.empty()) {
				auto& info = waiting.front();
				get_or_open(info.name, info.embedded_data, info.embedded_size);
				waiting.pop();
			}
		}*/
		void AsyncRequire(const Key& key, void* ed = nullptr, size_t es = 0, const Path& fp= {});
		inline void NotifyTasker() {
			tasker._queueCv.notify_one();
			tasker.startWorkerIfNeeded();
		}
		Effect* LutSwitch(int type) {
			const auto entry = lut.getEntryByType(type);
			if (!entry) return nullptr;
			Effect* fx = get(entry->effectName);
			if (fx && fx->Switch(entry->techIndex, entry->passIndex)) return fx;
			return nullptr;
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






	extern CRITICAL_SECTION& g_D3DContextLock;
	extern HMODULE hModule;
}
