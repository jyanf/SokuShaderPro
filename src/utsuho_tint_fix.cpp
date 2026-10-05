#include "utsuho_tint_fix.hpp"
namespace {
	using SokuLib::SpriteEx;
	using PF = void (SpriteEx::*)(float a, float r, float g, float b);
	using EM = spr::EffectManager;
	//constexpr DWORD ADDR_UTSUHO_TINT = 0x7d5b8f;
	//inline PF ogDrawTint ;//0x7fb150;

	constexpr DWORD ADDR_SWITCH_TINT_CASE = 0x7b5e58;
	inline DWORD ogJMPDrawTint = 0x7b5d0e;

	inline bool __fastcall tryDrawUtsuhoTint(SokuLib::v2::PlayerUtsuho* This, int _, float x, float y, float w, float h) {
		auto effect = EM::g_EffectBattle.effect;
#ifdef _DEBUG
		//check fx and pass
		if (!effect) return false;
		D3DXHANDLE cur;
		if (!(cur = effect->GetCurrentTechnique())) return false;
		D3DXTECHNIQUE_DESC desc;
		if (FAILED(effect->GetTechniqueDesc(cur, &desc))) return false;
		if (desc.Passes < 8) return false;
#endif
		SokuLib::textureMgr.setTexture(This->capeTexture, 1);
		effect->SetFloat("x", x);
		effect->SetFloat("y", y);
		effect->SetFloat("width", w);
		effect->SetFloat("height", h);
		SokuLib::DrawUtils::DxSokuColor shaderColor = This->renderInfos.shaderColor;
		D3DXVECTOR4 tcolor{
			static_cast<FLOAT>(shaderColor.r / 255.0),
			static_cast<FLOAT>(shaderColor.g / 255.0),
			static_cast<FLOAT>(shaderColor.b / 255.0),
			static_cast<FLOAT>(shaderColor.a / 255.0),
		};
		effect->SetVector("v4Arg0", &tcolor);
		effect->CommitChanges();
		effect->Begin(nullptr, D3DXFX_DONOTSAVESAMPLERSTATE);
		auto ret = effect->BeginPass(7);//P7
		//void __fastcall CSpriteEx_DrawRectSpriteWithVS(CSpriteEx * This);
		reinterpret_cast<void(__fastcall*)(SokuLib::SpriteEx*)>(0x4076b0)(&This->sprite);
		effect->EndPass();
		effect->End();
		return SUCCEEDED(ret);
	}
	/***********************************************************************************
		...
		007b5c18 d9  5c  24  10		FSTP       dword ptr [ESP  + local_8 ]
		007b5c1c d9  86  98			FLD        dword ptr [ESI  + 0x898 ]
				 08  00  00
		007b5c22 db  44  24  0c		FILD       dword ptr [ESP  + local_c ]
		007b5c26 d8  86  f0			FADD       dword ptr [ESI  + 0xf0 ]
				 00  00  00
		007b5c2c de  e9				FSUBP
		007b5c2e d9  5c  24  0c		FSTP       dword ptr [ESP  + local_c ]
		007b5c32 db  44  24  08		FILD       dword ptr [ESP  + local_10 ]
		007b5c36 da  4c  24  04		FIMUL      dword ptr [ESP  + local_14 ]
		007b5c3a 89  44  24  04		MOV        dword ptr [ESP  + local_14 ],EAX
		007b5c3e 8b  86  14			MOV        EAX ,dword ptr [ESI  + 0x114 ]
				 01  00  00
		007b5c44 83  f8  03			CMP        EAX ,0x3
		007b5c47 dd  05  10			FLD        qword ptr [DAT_00859910 ]
				 99  85  00
		007b5c4d dc  c9				FMUL       ST1
		007b5c4f d9  c9				FXCH
		007b5c51 d9  5c  24  08		FSTP       dword ptr [ESP  + local_10 ]
		007b5c55 db  44  24  04		FILD       dword ptr [ESP  + local_14 ]
		007b5c59 de  c9				FMULP
		007b5c5b d9  5c  24  04		FSTP       dword ptr [ESP  + local_14 ]
		...
		007b5c65 ff  24  85			JMP        dword ptr [EAX *0x4 + ->switchD_007b5c65::case0]
				 50  5e  7b  00
	***********************************************************************************/
	constexpr DWORD jumper_end = 0x7b5d94;
	inline __declspec(naked) void jumper() {
		_asm {
			//if call(esp)
			mov ecx, esi
			//get calculated floats
			push[esp + 0x4]//h
			push[esp + 0xC]//w
			push[esp + 0x14]//y
			push[esp + 0x1C]//x
			call tryDrawUtsuhoTint;
			test eax, eax
				je _retry
				//jmp end
				jmp jumper_end
				//else jmp org
				_retry :
			jmp dword ptr[ogJMPDrawTint]
		}
	}
}

namespace spr {
	void Hook_DrawUtsuhoTint() {
		ogJMPDrawTint = SokuLib::TamperDword(ADDR_SWITCH_TINT_CASE, (DWORD)&jumper);
	}
}