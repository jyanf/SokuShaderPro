#include "main.hpp"

#include "fx_SpellBgBlend.h"
#include "fx_Battle.h"
#include "fx_UtsuhoLimited.h"

#include "SokuLib.hpp"

#include "debug_helper.hpp"
#include "utsuho_tint_fix.hpp"
#include "battle_ex.hpp"

namespace spr {
	using EM = EffectManager;
	
	CBaseEffect& EM::g_EffectBattle = *reinterpret_cast<CBaseEffect*>(0x89aafc);
	//Fun_CreateEffect ogCreateEffect = nullptr;
	volatile bool Effect::_begined = false;
	auto& g_D3DContextLock = *reinterpret_cast<CRITICAL_SECTION*>(0x8a0e14);

	decltype(&EM::OnClose) EM::ogOnClose = nullptr;
	decltype(&ID3DXEffect::End) ogEffectEnd = nullptr;
	static __declspec(nothrow) HRESULT __stdcall EndWithModifiedTexture(ID3DXEffect* This) {
		auto ret = (This->*ogEffectEnd)();
		auto pdev = SokuLib::pd3dDev; 
		//This->GetDevice(&pdev);
		auto& stagedEID = *reinterpret_cast<unsigned int(*)[8]>((DWORD)&SokuLib::textureMgr + 0x64);
		for (int i = 0; i < 8; ++i) {
			auto handle = stagedEID[i] ? *SokuLib::textureMgr.Get(stagedEID[i]) : NULL;
			pdev->SetTexture(i, handle);
		}
		return ret;
	}
	static void ReplaceIVtable(ID3DXEffect* This) {
		constexpr size_t VTABLE_SIZE = 79;
		static void** targetEffectVtable = nullptr;//nvm for this trivial leak
		auto& vtable = *reinterpret_cast<void***>(This);
		if (!targetEffectVtable) {
			targetEffectVtable = new void* [VTABLE_SIZE];
			memmove(targetEffectVtable, vtable, sizeof(void*[VTABLE_SIZE]));
			*reinterpret_cast<void**>(&ogEffectEnd) = SokuLib::TamperDword(&targetEffectVtable[67], EndWithModifiedTexture);
		}
		vtable = (void**)targetEffectVtable;
	}
	//class StateManager : public ID3DXEffectStateManager {
	//	STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID* ppv) override {
	//		//return IUnknown_QueryInterface_Proxy(this, iid, ppv);
	//		if (!ppv) return E_POINTER;
	//		*ppv = nullptr;
	//		if (iid == IID_IUnknown || iid == IID_ID3DXEffectStateManager) {
	//			*ppv = static_cast<ID3DXEffectStateManager*>(this);
	//			AddRef();
	//			return S_OK;
	//		}
	//		return E_NOINTERFACE;
	//	}
	//	STDMETHOD_(ULONG, AddRef)(THIS) override {
	//		return 1;
	//	}
	//	STDMETHOD_(ULONG, Release)(THIS) override {
	//		return 1;
	//	}
	//	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) override { return SokuLib::pd3dDev->SetTransform(State, pMatrix); }
	//	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial) override { return SokuLib::pd3dDev->SetMaterial(pMaterial); }
	//	STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9* pLight) override { return SokuLib::pd3dDev->SetLight(Index, pLight); }
	//	STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable) override { return SokuLib::pd3dDev->LightEnable(Index, Enable); }
	//	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value) override { return SokuLib::pd3dDev->SetRenderState(State, Value); }
	//	STDMETHOD(SetTexture)(THIS_ DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture) override { return SokuLib::pd3dDev->SetTexture(Stage, pTexture); }
	//	STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) override { return SokuLib::pd3dDev->SetTextureStageState(Stage, Type, Value); }
	//	STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) override { return SokuLib::pd3dDev->SetSamplerState(Sampler, Type, Value); }
	//	STDMETHOD(SetNPatchMode)(THIS_ FLOAT NumSegments) override { return SokuLib::pd3dDev->SetNPatchMode(NumSegments); }
	//	STDMETHOD(SetFVF)(THIS_ DWORD FVF) override { return SokuLib::pd3dDev->SetFVF(FVF); }
	//	STDMETHOD(SetVertexShader)(THIS_ LPDIRECT3DVERTEXSHADER9 pShader) override { return SokuLib::pd3dDev->SetVertexShader(pShader); }
	//	STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT RegisterIndex, CONST FLOAT* pConstantData, UINT RegisterCount) override { return SokuLib::pd3dDev->SetVertexShaderConstantF(RegisterIndex, pConstantData, RegisterCount); }
	//	STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT RegisterIndex, CONST INT* pConstantData, UINT RegisterCount) override { return SokuLib::pd3dDev->SetVertexShaderConstantI(RegisterIndex, pConstantData, RegisterCount); }
	//	STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT RegisterIndex, CONST BOOL* pConstantData, UINT RegisterCount) override { return SokuLib::pd3dDev->SetVertexShaderConstantB(RegisterIndex, pConstantData, RegisterCount); }
	//	STDMETHOD(SetPixelShader)(THIS_ LPDIRECT3DPIXELSHADER9 pShader) override { return SokuLib::pd3dDev->SetPixelShader(pShader); }
	//	STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT RegisterIndex, CONST FLOAT* pConstantData, UINT RegisterCount) override { return SokuLib::pd3dDev->SetPixelShaderConstantF(RegisterIndex, pConstantData, RegisterCount); }
	//	STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT RegisterIndex, CONST INT* pConstantData, UINT RegisterCount) override { return SokuLib::pd3dDev->SetPixelShaderConstantI(RegisterIndex, pConstantData, RegisterCount); }
	//	STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT RegisterIndex, CONST BOOL* pConstantData, UINT RegisterCount) override { return SokuLib::pd3dDev->SetPixelShaderConstantB(RegisterIndex, pConstantData, RegisterCount); }
	//	//static void PrePass() {}//manually save
	//	//static void PostPass() {}//manually restore
	//} Effect::smg;
	
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
	if (path.empty()) {
		std::cout << DYELLOW
			<< "Cannot found target file of shader \""<<fxname<<"\", fallback to embedded binary."
			<< DORG << std::endl;
	} else {
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
		if (SUCCEEDED(ret) && result.effect) {
			if constexpr (!EFFECT_BEGIN_FLAG) ReplaceIVtable(result.effect);
			return ret;
		}
#endif
		if (_errMsg) {
			std::cout << DRED
				<< "Failed to load/compile target: \n\t" << fxname << std::endl
				<< "\t" << std::string_view((const char*)_errMsg->GetBufferPointer(), _errMsg->GetBufferSize())
				<< DYELLOW
				<< "Fallback to embedded binary."
				<< DORG << std::endl;
			_errMsg->Release(); 
		}
	}
	//use embedded
	if (pdata) ret = D3DXCreateEffect(SokuLib::pd3dDev, pdata, psize, nullptr, nullptr, compiler_flag, nullptr, &result.effect, &_errMsg);
	if (SUCCEEDED(ret) && result.effect) {
		if constexpr (!EFFECT_BEGIN_FLAG) ReplaceIVtable(result.effect);
	} else if (_errMsg) {
		std::cout << DRED
			<< "Failed to load/compile target: \n\t" << fxname << std::endl
			<< "\t" << std::string_view((const char*)_errMsg->GetBufferPointer(), _errMsg->GetBufferSize())
			<< DORG << std::endl;
		_errMsg->Release();
	}
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
		EM::instance().NotifyTasker(600);//wait for other submit
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
	//this->debug(ret);//don't need you for now
	return ret == D3D_OK;
}

	static void Tamper_FixBeginSaveFlag() {
		constexpr byte fixflag = EFFECT_BEGIN_FLAG;
		constexpr DWORD flags[] = {
			0x470496, // Spell Background
			0x7fb10f, // Draw Add
			0x7fb1b7, // Draw Tint
			0x7fb254, // Draw Gray
			0x7fb302, // Draw Utsuho
			0x7fb43b, // Draw Utsuho Add
			0x7fb528, // Draw Utsuho Gray
		};

		// new d3dxeffect runtime restores staged texture with sampler state
		// which conflicts with game's org texture stage buffer mechanism
		for (auto p : flags) *(byte*)p |= fixflag;
		std::cout << "All ID3DXEffect::Begin() flag tampered." << std::endl;
	}
/*****************************    TotalInit    ********************************/
void Initialize() {
	//spell bg
	Effect::OrgCreateEffect<0>_v0 = SokuLib::TamperNearCall(0x471665, &Effect::MyCreateEffect<0>);
	//battle common
	Effect::OrgCreateEffect<1>_v1 = SokuLib::TamperNearCall(0x7fb030, &Effect::MyCreateEffect<1>);
	//utsuho cape limited render (unused)
	Effect::OrgCreateEffect<2>_v2 = SokuLib::TamperNearCall(0x7fb049, &Effect::MyCreateEffect<2>);
	if constexpr (EFFECT_BEGIN_FLAG) {//old fix method
		Tamper_FixBeginSaveFlag();
	}
	// 
	//on soku close
	EffectManager::ogOnClose = SokuLib::TamperNearCall(0x440568, &EffectManager::OnClose);

	Hook_DrawUtsuhoTint();
	Hook_ObjOnRenderEnd();
}



std::filesystem::path GetShaderFolder() {
	return GetModuleFolder() / "shaders";
}












}