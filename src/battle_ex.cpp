#include "battle_ex.hpp"
namespace {
	constexpr DWORD ADDR_JA_DEFAULT_CASE = 0x43904a;
	inline DWORD ogJaDefault = 0x439179;
	using EM = spr::EffectManager;

	inline bool __fastcall tryMoreShaderTypes(SokuLib::v2::AnimationObject* This) {
		int shaderType = *reinterpret_cast<int*>(&This->renderInfos.shaderType);
		auto myfx = EM::instance().get("BattleEx");
		if (!myfx || !myfx->effect) return false;
		auto& ifx = *myfx->effect;
		switch (shaderType) {
			//case -SokuLib::v2::BlendOptions::NORMAL: {
		case -1: case -2: case -3: case -4: {
			ifx.SetTechnique("BlendOverrides");
			//save render state
			ifx.Begin(nullptr, 0); ifx.BeginPass(-shaderType - 1);
			//draw
			This->sprite.render();
			//restore
			ifx.EndPass(); ifx.End();
		} break;
		default:
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
		EM::instance().require("BattleEx", (void*)fx_BattleEx_bytecode, sizeof(fx_BattleEx_bytecode));
		ogJaDefault = SokuLib::TamperNearJmpOpr(ADDR_JA_DEFAULT_CASE+1, (DWORD)&jumper);
	}
}