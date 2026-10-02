#include "main.hpp"

#include "fx_SpellBgBlend.h"
#include "fx_Battle.h"
#include "fx_UtsuhoLimited.h"

#include "SokuLib.hpp"

namespace spr {
	
	//Fun_CreateEffect ogCreateEffect = nullptr;
	
	void AddListenerWrapper(void* obj) {
		constexpr DWORD dxContext = 0x8a0e10;
		constexpr uintptr_t addr = 0x4153A0;
		__asm
		{
			mov eax, dxContext
			push obj
			call addr
		}
	}

static std::filesystem::path get_shader_file(const char* file) {
	auto base = GetShaderFolder() / file;
	if (!base.has_extension()) base.replace_extension(".fx");
	if (std::filesystem::is_regular_file(base)) {
		return base;
	} else return {};
}
static HRESULT GetEffectWarm(CEffect& result, const char* filename, void* pdata, size_t psize) {
	HRESULT ret = 1;
	if (result.effect) return ret;
	AddListenerWrapper(&result);
	ID3DXEffectCompiler* _compiler = nullptr;
	ID3DXBuffer* _errMsg = nullptr;

	//note string compiler D3DXCreateEffectCompiler
	
	//note ps_1_1 D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
	const DWORD compiler_flag = D3DXSHADER_OPTIMIZATION_LEVEL1 | D3DXSHADER_SKIPVALIDATION | D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
	auto path = get_shader_file(filename);
	if (!path.empty()) {
		std::wcout << "\x1b[33m"
			<< "Compile D3DXEffect from file: \n\t" << path.c_str() 
			<< "\x1b[0m" << std::endl;
#if 0
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
#else
		ret = D3DXCreateEffectFromFileW(SokuLib::pd3dDev, path.c_str(), 
			nullptr, nullptr, compiler_flag, 0, &result.effect, &_errMsg);
		if (SUCCEEDED(ret) && result.effect) return ret;//build successfully
#endif
	}
	if (_errMsg) { 
		std::cout << "\x1b[31m"//red
			<< "Failed to compile target: \n\t" << filename << std::endl
			<< "\t" << std::string_view((const char*)_errMsg->GetBufferPointer(), _errMsg->GetBufferSize()) << std::endl
			<< "Fallback to embedded binary."
			<< "\x1b[0m" << std::endl;
		_errMsg->Release(); 
	}
	ret = D3DXCreateEffect(SokuLib::pd3dDev, pdata, psize, nullptr, nullptr, 0, nullptr, &result.effect, &_errMsg);
	return ret;
}
template <auto FX>
bool CEffect::CreateEffect(void* pdata, size_t psize) {
	
	HRESULT ret = 1;
	if constexpr (FX == 0) {
		ret = GetEffectWarm(*this, "SpellBgBlend", (void*)fx_SpellBgBlend_bytecode, sizeof(fx_SpellBgBlend_bytecode));
	} else if constexpr (FX == 1) {
		ret = GetEffectWarm(*this, "Battle", (void*)fx_Battle_bytecode, sizeof(fx_Battle_bytecode));
	} else if constexpr (FX == 2) {
		ret = GetEffectWarm(*this, "UtsuhoLimited", (void*)fx_UtsuhoLimited_bytecode, sizeof(fx_UtsuhoLimited_bytecode));
	} else {
		return (this->*OrgCreateEffect<FX>{})(pdata, psize);
	}
#ifdef _DEBUG
	if (SUCCEEDED(ret)) {
		D3DXEFFECT_DESC effectDesc;
		this->effect->GetDesc(&effectDesc);
		puts("Parameters Desc:");
		for (UINT i = 0; i < effectDesc.Parameters; ++i) {
			D3DXHANDLE h = this->effect->GetParameter(NULL, i);
			D3DXPARAMETER_DESC desc;
			this->effect->GetParameterDesc(h, &desc);
			printf("\t%s type=%d class=%d\n", desc.Name, desc.Type, desc.Class);
		}
		puts("");
	}
#endif // _DEBUG
	return ret == S_OK;
}


void Initialize() {
	//spell bg
	OrgCreateEffect<0>_v0 = SokuLib::TamperNearCall(0x471665, &CEffect::CreateEffect<0>);
	//battle common
	OrgCreateEffect<1>_v1 = SokuLib::TamperNearCall(0x7fb030, &CEffect::CreateEffect<1>);
	//utsuho cape limited render (unused)
	OrgCreateEffect<2>_v2 = SokuLib::TamperNearCall(0x7fb049, &CEffect::CreateEffect<2>);

}



std::filesystem::path GetShaderFolder() {
	return GetModuleFolder() / "shaders";
}












}