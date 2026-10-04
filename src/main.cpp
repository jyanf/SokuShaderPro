#include "main.hpp"

#include "fx_SpellBgBlend.h"
#include "fx_Battle.h"
#include "fx_UtsuhoLimited.h"

#include "SokuLib.hpp"

#include "debug_helper.hpp"
#include "utsuho_tint_fix.hpp"
#include "battle_ex.hpp"

namespace spr {
	//EffectManager EffectManager::instance;
	using EM = EffectManager;
	CBaseEffect& EM::g_EffectBattle = *reinterpret_cast<CBaseEffect*>(0x89aafc);
	//Fun_CreateEffect ogCreateEffect = nullptr;
	volatile bool Effect::_begined = false;
	auto& g_D3DContextLock = *reinterpret_cast<CRITICAL_SECTION*>(0x8a0e14);
	
static std::filesystem::path get_effect_file(const char* file) {
	using std::filesystem::path, std::filesystem::is_regular_file;
	auto base = GetShaderFolder() / file;
	//if (base.has_extension()) {
	//	return is_regular_file(base) ? base : path{};
	//}
	
	if (std::filesystem::is_regular_file(base.replace_extension(".fx")))
		return base;

	if (std::filesystem::is_regular_file(base.replace_extension(".fxo")))
		return base;

	return {};
}
HRESULT Effect::CreateEffectWarm(Effect& result, const char* fxname, void* pdata, size_t psize, const std::filesystem::path& filepath) {
	HRESULT ret = 1;
	ID3DXEffectCompiler* _compiler = nullptr;
	ID3DXBuffer* _errMsg = nullptr;

	//note string compiler D3DXCreateEffectCompiler
	
	const DWORD compiler_flag = 0
		| D3DXSHADER_OPTIMIZATION_LEVEL1 | D3DXSHADER_SKIPVALIDATION 
			| D3DXSHADER_USE_LEGACY_D3DX9_31_DLL //ps_1_1 support
		| D3DXFX_NOT_CLONEABLE //btw never D3DXFX_LARGEADDRESSAWARE
		;
	auto& path = (filepath.empty() || !std::filesystem::is_regular_file(filepath)) ? get_effect_file(fxname) : filepath;
	if (!path.empty()) {
		std::wcout << DYELLOW
			<< "Loading/Compiling D3DXEffect from file: \n\t" << path.c_str() 
			<< DORG << std::endl;
#if 0 //async?
		ret = D3DXCreateEffectCompilerFromFileW(path.c_str(), nullptr, nullptr, compiler_flag, &_compiler, &_errMsg);
		if (SUCCEEDED(ret) && _compiler) {
			ID3DXBuffer* _buffer;
			ret = _compiler->CompileEffect(compiler_flag, &_buffer, &_errMsg);
			if (SUCCEEDED(ret) && _buffer) {
				ret = D3DXCreateEffect(SokuLib::pd3dDev, _buffer->GetBufferPointer(), _buffer->GetBufferSize(),
					nullptr, nullptr, 0, 0, &result.effect, &_errMsg);
				if (SUCCEEDED(ret) && result.effect) return ret;//build successfully
				_buffer->Release();
			}
			_compiler->Release();
		}
#else //sync
		ret = D3DXCreateEffectFromFileW(SokuLib::pd3dDev, path.c_str(), 
			nullptr, nullptr, compiler_flag, 0, &result.effect, &_errMsg);
		if (SUCCEEDED(ret) && result.effect) return ret;//build successfully
#endif
	}
	if (_errMsg) {
		std::cout << DRED
			<< "Failed to load/compile target: \n\t" << fxname << std::endl
			<< "\t" << std::string_view((const char*)_errMsg->GetBufferPointer(), _errMsg->GetBufferSize())
			<< DYELLOW
			<< "Fallback to embedded binary."
			<< DORG << std::endl;
		_errMsg->Release(); 
	}
	//use embedded
	if (pdata) ret = D3DXCreateEffect(SokuLib::pd3dDev, pdata, psize, nullptr, nullptr, compiler_flag, nullptr, &result.effect, &_errMsg);
	return ret;
}
void Effect::debug(HRESULT result) {
#ifdef _DEBUG
	fx_debug(this, result);
#endif // _DEBUG
}
template <auto FX>
bool Effect::MyCreateEffect(void* pdata, size_t psize) {
	if (this->effect) return false;
	HRESULT ret = 1;
	if constexpr (FX == 0) {
		Effect::AddListenerWrapper(this);
		ret = CreateEffectWarm(*this, "SpellBgBlend", (void*)fx_SpellBgBlend_bytecode, sizeof(fx_SpellBgBlend_bytecode));
	} else if constexpr (FX == 1) {
		//extras
		EM::instance().NotifyTasker();
		//org
		Effect::AddListenerWrapper(this);
		ret = CreateEffectWarm(*this, "Battle", (void*)fx_Battle_bytecode, sizeof(fx_Battle_bytecode));
	} else if constexpr (FX == 2) {
		Effect::AddListenerWrapper(this);
		ret = CreateEffectWarm(*this, "UtsuhoLimited", (void*)fx_UtsuhoLimited_bytecode, sizeof(fx_UtsuhoLimited_bytecode));
	} else if (OrgCreateEffect<FX>{}) {
		//no AddListenerWrapper cuz already called in org
		return (this->*OrgCreateEffect<FX>{})(pdata, psize);
	}
	this->debug(ret);
	return ret == D3D_OK;
}

	decltype(&EffectManager::OnClose) EffectManager::ogOnClose = nullptr;
void Initialize() {
	//spell bg
	Effect::OrgCreateEffect<0>_v0 = SokuLib::TamperNearCall(0x471665, &Effect::MyCreateEffect<0>);
	//battle common
	Effect::OrgCreateEffect<1>_v1 = SokuLib::TamperNearCall(0x7fb030, &Effect::MyCreateEffect<1>);
	//utsuho cape limited render (unused)
	Effect::OrgCreateEffect<2>_v2 = SokuLib::TamperNearCall(0x7fb049, &Effect::MyCreateEffect<2>);

	//on soku close
	EffectManager::ogOnClose = SokuLib::TamperNearCall(0x440568, &EffectManager::OnClose);

	Hook_DrawUtsuhoTint();
	Hook_ObjOnRenderEnd();
}



std::filesystem::path GetShaderFolder() {
	return GetModuleFolder() / "shaders";
}












}