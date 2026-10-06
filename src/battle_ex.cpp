#include "battle_ex.hpp"
namespace {
	constexpr DWORD ADDR_JA_DEFAULT_CASE = 0x43904a;
	inline DWORD ogJaDefault = 0x439179;
	using EM = spr::EffectManager;

	inline bool __fastcall tryMoreShaderTypes(SokuLib::v2::AnimationObject* This) {
		int shaderType = *reinterpret_cast<int*>(&This->renderInfos.shaderType);
		switch (shaderType) {
			//case -SokuLib::v2::BlendOptions::NORMAL: {
		case -1: case -2: case -3: case -4: {
			auto myfx = EM::instance().get("BattleEx");
			if (!myfx) return false;
			myfx->Switch("BlendOverrides", -shaderType-1);
			{ auto guard = myfx->GetRenderGuard(); This->sprite.render(); }
		} break;
		default://custom part
			if (-RESERVE_SHADER_COUNT <= shaderType && shaderType < 0) return false;
			auto fx = EM::instance().LutSwitchShader(shaderType);
			if (fx) {
				//hasty
				SokuLib::DrawUtils::DxSokuColor sc = This->renderInfos.shaderColor;
				D3DXVECTOR4 scf {
					sc.r / 255.0f, sc.g / 255.0f, sc.b / 255.0f, sc.a / 255.0f
				};
				if (SUCCEEDED(fx->effect->SetVector("ShaderColor", &scf))) {
					fx->effect->CommitChanges();
				}
				//hasty
				//add callback here?
				auto _ = fx->GetRenderGuard();
				This->sprite.render();
			}

			return false;
		}
		return true;
	}
	constexpr DWORD jumper_end = 0x439179;
	inline __declspec(naked) void jumper() {
		_asm {
			//if call
			call tryMoreShaderTypes;
			test eax, eax
				je _skip
				//jmp end
				jmp jumper_end
				//else jmp org
				_skip :
			jmp dword ptr[ogJaDefault]
		}
	}
}
namespace spr {
	void Hook_ObjOnRenderEnd() {
		EM::instance().AsyncRequire("BattleEx", (void*)fx_BattleEx_bytecode, sizeof(fx_BattleEx_bytecode));
		ogJaDefault = SokuLib::TamperNearJmpOpr(ADDR_JA_DEFAULT_CASE+1, (DWORD)&jumper);
	}
}