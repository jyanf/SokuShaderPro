#pragma once
#include "main.hpp"
#include "debug_helper.hpp"
namespace {
	using EM = spr::EffectManager;
	using spr::Effect;


	inline static std::string join(const std::string& a, const std::string& b, const std::string& c) {
		return a + "::" + b + "::" + c;
	}

	inline static D3DXHANDLE GetNextTechnique(ID3DXEffect* dx, D3DXHANDLE cur) {
		D3DXHANDLE hTech = NULL;
		if (FAILED(dx->FindNextValidTechnique(cur, &hTech))) return NULL;
		return hTech;
	}

	static std::vector<std::string> splitURI(const std::string_view& uri) {
		std::vector<std::string> out;
		std::string s = uri.data();
		for (char& c : s) {
			switch (c) {
			case '/': case '\\': case ':': case '.':
				c = '\n';
				break;
			default:
				break;
			}
		}
		std::istringstream iss(s);
		std::string tok;
		while (std::getline(iss, tok, '\n')) {
			if (!tok.empty()) out.push_back(tok);
		}
		return out;
	}
}
	

void EM::LUT::registerEffect(const std::string& effectName, Effect* eff) {
	if (!eff || !eff->check()) return;
	ID3DXEffect* dx = eff->effect;

	std::lock_guard<std::mutex> lk(_mtx);
	std::cout << DYELLOW 
		<< "LUT.registerEffect: begin registering effect [" + effectName + "]" 
		<< DORG << std::endl;

	auto existing = _effectToIds.find(effectName);
	if (existing != _effectToIds.end()) {
		std::cout << DRED
			<< "LUT.registerEffect: found existing mappings for [" + effectName + "], removing " + std::to_string(existing->second.size()) + " ids"
			<< DORG << std::endl;
		for (int id : existing->second) {
			_idToEntry.erase(id);
		}
		existing->second.clear();
	}

	// enumerate techniques & passes
	int techIdx = 0;
	D3DXHANDLE hTech = NULL;
	while (hTech = GetNextTechnique(dx, hTech)) {
		D3DXTECHNIQUE_DESC tdesc;
		if (FAILED(dx->GetTechniqueDesc(hTech, &tdesc))) {
			std::cout <<DRED 
				<<"LUT.registerEffect: GetTechniqueDesc failed. tech index " + std::to_string(techIdx)
				<< DORG <<std::endl;
			++techIdx;
			continue;
		}
		std::string techName = tdesc.Name ? tdesc.Name : std::to_string(techIdx);
		std::cout 
			<< "LUT.registerEffect: discovered technique idx=" + std::to_string(techIdx) + " name='" + techName + "' passes=" + std::to_string(tdesc.Passes)
			<< std::endl;
		UINT passCount = tdesc.Passes;
		for (UINT p = 0; p < passCount; ++p) {
			D3DXHANDLE hPass = dx->GetPass(hTech, p);
			D3DXPASS_DESC pdesc;
			std::string passName;
			if (SUCCEEDED(dx->GetPassDesc(hPass, &pdesc))) {
				passName = pdesc.Name ? pdesc.Name : std::to_string(p);
			} else {
				passName = std::to_string(p);
			}

			int id = _allocId();
			int passIdx = static_cast<int>(p);
			_idToEntry.emplace(std::piecewise_construct,
				std::forward_as_tuple(id), 
				std::forward_as_tuple(effectName, techIdx, techName, passIdx, passName)
			);
			_effectToIds[effectName].push_back(id);
			std::cout<< DCYAN 
				<< "LUT.registerEffect: mapped shaderType=" << DBOLD << std::to_string(id) << "; "
				<< DORG << DBLUE
				<< "uri=\"" + effectName + "." 
					+ techName + "(" + std::to_string(techIdx) + ")." 
					+ passName + "(" + std::to_string(passIdx) + ")\""
				<< DORG << std::endl;
			//flattened uri
			//	effect::techIndex::passIndex
			//	effect::techName::passIndex
			//	effect::techIndex::passName
			//	effect::techName::passName
			std::string k1 = join(effectName, std::to_string(techIdx), std::to_string(passIdx));
			std::string k2 = join(effectName, techName, std::to_string(passIdx));
			std::string k3 = join(effectName, std::to_string(techIdx), passName);
			std::string k4 = join(effectName, techName, passName);

			_keyToId.emplace(k1, id);
			_keyToId.emplace(k2, id);
			_keyToId.emplace(k3, id);
			_keyToId.emplace(k4, id);
		}
		++techIdx;
	}
	std::cout << DGREEN
		<< "LUT.registerEffect: finished registering effect.\n"
		<< DORG << std::endl;
}
//get shaderType by uri string
//	e.g. "effect/tech/pass" »ò "effect.tech.pass" »ò "effect:tech:pass"
//	tech/pass could be index number or name
std::optional<int> EM::LUT::getShaderTypeByURI(const std::string_view& uri) {
	auto parts = splitURI(uri);
	if (parts.size() != 3) return std::nullopt;
	std::lock_guard<std::mutex> lk(_mtx);
	
	std::string k = join(parts[0], parts[1], parts[2]);
	auto it = _keyToId.find(k);
	if (it != _keyToId.end()) return it->second;
	std::cout << DRED 
		<< "LUT.getShaderTypeByURI: invalid uri '" << uri
		<< DORG << std::endl;
	return std::nullopt;
}

// look up shader approach by shaderType number
const EM::LUT::Entry* EM::LUT::getEntryByType(int shaderType) {
	std::lock_guard<std::mutex> lk(_mtx);
	auto it = _idToEntry.find(shaderType);
	if (it == _idToEntry.end()) return nullptr;
	return &it->second;
}