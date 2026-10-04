#pragma once
#include <string>
#include <iostream>
#include <Windows.h>
#include <Shlwapi.h>
#include <filesystem>
#include <unordered_map>
#include <queue>

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

	const auto ADDR_C_BASE_EFFECT_VTABLE = (void**)0x871334;
	struct CBaseEffect {
		ID3DXEffect* effect = nullptr;
		inline virtual void OnLostDevice() {
			//if (effect) effect->OnLostDevice();
			return (this->*SokuLib::union_cast<void(CBaseEffect::*)()>(ADDR_C_BASE_EFFECT_VTABLE[0]))();
		}
		inline virtual void OnResetDevice() {
			//if (effect) effect->OnResetDevice();
			return (this->*SokuLib::union_cast<void(CBaseEffect::*)()>(ADDR_C_BASE_EFFECT_VTABLE[1]))();
		}
		inline ~CBaseEffect() {//non virtual, without unregister listener
			//if (effect) { effect->Release(); effect = nullptr; }
			reinterpret_cast<void(__fastcall*)(CBaseEffect*)>(0x418160)(this);
		}
	};
	class Effect : public CBaseEffect {
		using Base = CBaseEffect;
	public:
		bool enabled = false;

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
		static HRESULT CreateEffectWarm(Effect& result, const char* filename, void* pdata, size_t psize);
		
		void debug(HRESULT result);
		inline Effect(const std::string_view& name, void* embedded_data=nullptr, size_t embedded_size=0) {
			AddListenerWrapper(this);
			auto ret = CreateEffectWarm(*this, name.data(), embedded_data, embedded_size);
			enabled = ret == S_OK;
			debug(ret);
		}
		inline ~Effect() {//with unregister
			//RemoveListenerWrapper(this); CBaseEffect::~CBaseEffect();
			reinterpret_cast<void(__fastcall*)(CBaseEffect*)>(0x4181e0)(this);
		}
	};
	class EffectManager : std::unordered_map<std::string_view, Effect*> {
		using Key = value_type::first_type;
		using Value = value_type::second_type;
		using Base = std::unordered_map<Key, Value>;
		//singleton
		EffectManager() = default;
		EffectManager(const EffectManager&) = delete; EffectManager(EffectManager&&) = delete;
		EffectManager& operator=(const EffectManager&) = delete; EffectManager& operator=(EffectManager&&) = delete;
		inline virtual ~EffectManager() {
			for (auto& [k, v] : (*this)) {
				delete (CBaseEffect*)v;//should not remove listener cuz the whole game is closing
			}
		}
		struct FXInfo {
			Key name;
			void* embedded_data = nullptr;
			size_t embedded_size = 0;
		};
		std::queue<FXInfo> waiting;
		// tech pass tree map
	public:
		static CBaseEffect& g_EffectBattle;
		//singleton
		static EffectManager& instance() {
			static EffectManager instance;
			return instance;
		}
		inline static void OnClose() {
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
		inline Value& get_or_open(Key key, void* ed=nullptr, size_t es=0) {
			auto [it, inserted] = this->emplace(key, nullptr);
			if (inserted) {
				it->second = new Effect(key, ed, es);
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

		void require(Key key, void* ed = nullptr, size_t es = 0) {
			waiting.emplace(key, ed, es);
		}
		void open_all() {
			while (!waiting.empty()) {
				auto& info = waiting.front();
				get_or_open(info.name, info.embedded_data, info.embedded_size);
				waiting.pop();
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







	extern HMODULE hModule;
}
