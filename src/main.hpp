#pragma once
#include <string>
#include <iostream>
#include <map>
#include <Windows.h>
#include <Shlwapi.h>
#include <filesystem>

#include "d3d9.h"
#include "d3dx9.h"
#include "d3dx9effect.h"

namespace spr {
	constexpr DWORD SHADER0_BYTECODE_OFFSET = 0x85a608;
	constexpr size_t SHADER0_BYTECODE_SIZE = 0x4d0;
	constexpr DWORD SHADER1_BYTECODE_OFFSET = 0x85f908;
	constexpr size_t SHADER1_BYTECODE_SIZE = 0x1b20;
	constexpr DWORD SHADER2_BYTECODE_OFFSET = 0x861428;
	constexpr size_t SHADER2_BYTECODE_SIZE = 0x684;

	using CEffect = struct _CBaseEffect {
		void* vtable;
		ID3DXEffect* effect;
		template<auto FX> bool CreateEffect(void* pdata, size_t psize);
	};

	template<auto FX>
	struct OrgCreateEffect {
		using PF = decltype(&CEffect::CreateEffect<FX>);
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
