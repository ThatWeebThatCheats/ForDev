#pragma once
#include "../../Valve_SDK/csgostructs.hpp"

class CChams : public Singleton<CChams>
{
public: 
	IMaterial* material_norm;
	IMaterial* material_norm_invis;

	void SceneEnd();
	void Initialize();
	void Shutdown();
private:
	bool TeamCheck(C_BasePlayer * ent);
};
