#pragma once
#include "../../Valve_SDK/csgostructs.hpp"
#include "../../hooks.hpp"

class CMisc : public Singleton<CMisc>
{
public:
	void SetNorecoilAngles(ClientFrameStage_t stage, QAngle* aim_punch, QAngle* view_punch, QAngle old_aim_punch, QAngle old_view_punch);
	void FakeDuck(CUserCmd * cmd, bool & bSendPacket);
	void NoVisualRecoil(ClientFrameStage_t stage, QAngle* aim_punch, QAngle* view_punch, QAngle old_aim_punch, QAngle old_view_punch);
	void SetThirdpersonAngles(ClientFrameStage_t stage);
	void FakeLag(CUserCmd * cmd, bool & bSendPacket);
	void SlowWalk(CUserCmd * cmd);
	void ClanTag();
	void AutoStrafe(CUserCmd * cmd);
	void ViewmodelChanger();
	void NoView(CViewSetup * vsView);
	void OverrideView(CViewSetup * vsView);
	void FastDuck(CUserCmd * cmd);
	void Bhop(CUserCmd* cmd);
	void AutoAccept(const char* pSoundEntry);
};