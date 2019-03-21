#pragma once
#include "../../Valve_SDK/csgostructs.hpp"

enum class YawAntiAims : int
{
	NONE,
	FREESTANDING,
	BACKWARDS
};

enum class PitchAntiAims : int
{
	NONE,
	EMOTION,
	DOWN,
	UP
};

class CAntiAim : public Singleton<CAntiAim>
{
public:
	void CreateMove(CUserCmd* cmd, bool& bSendPacket);
private:
	void Yaw(CUserCmd * cmd, bool & bSendPacket);
	void Pitch(CUserCmd * cmd);
	void DoAntiAim(CUserCmd * cmd, bool & bSendPacket);

	void Desync(CUserCmd* cmd, bool& bSendPacket);

	// Freestanding
	bool Freestanding(C_BasePlayer * player, CUserCmd * cmd);
	float fov_player(Vector ViewOffSet, QAngle View, C_BasePlayer* entity);
	int GetNearestPlayerToCrosshair();

	bool InLbyUpdate = false;
	bool NextTickInLbyUpdate = false;
	float NextLbyUpdate = 0.f;

	CBaseHandle* m_ulEntHandle = nullptr;
	CCSGOPlayerAnimState* m_serverAnimState = nullptr;

	bool allocate = false, change = false, reset = false;
};