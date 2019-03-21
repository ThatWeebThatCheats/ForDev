#pragma once
#include "../../Helpers/singleton.hpp"
#include "../../Valve_SDK/csgostructs.hpp"

struct PointScanStruct
{
	Vector pos;
	bool center = false;
};

struct CanHitStruct
{
	float damage = 0.f;
	bool CanHit = false;
	Vector pos = Vector(0, 0, 0);
};

enum class BaimMode : int
{
	NONE,
	BAIM,
	FORCE_BAIM
};

struct RbotMatrixData
{
	matrix3x4_t matrix[MAXSTUDIOBONES];
	bool gotMatrix = false;
	studiohdr_t* StudioHdr;
	mstudiohitboxset_t* StudioSet;
};

enum correction_flags
{
	DESYNC,
	NO_DESYNC,
	SLOW_WALK
};

struct extra
{
	int current_flag[64];
};

class CRageBot : public Singleton<CRageBot>
{
public:
	void CreateMove(CUserCmd* cmd, bool& bSendPacket);
	void AutoPistol(CUserCmd * cmd);
	bool GetBestHitboxPoint(C_BasePlayer* entity, float& damage, Vector& hitbox, BaimMode baim, bool& WillKill, matrix3x4_t matrix[MAXSTUDIOBONES] = nullptr, mstudiohitboxset_t* StudioSet = nullptr, bool NoPointscale = false);
	void missed_due_to_desync(IGameEvent * event);
	void Resolver(CUserCmd * cmd, C_BasePlayer* entity);
	void PVSFix(ClientFrameStage_t stage);
	void PrecacheShit();
	bool HitChance(QAngle angles, C_BasePlayer* ent, float chance);
private:
	bool InFakeLag(C_BasePlayer* player);
	float Simtimes[128];

	void AutoStop(CUserCmd* cmd);
	void AutoCrouch(CUserCmd* cmd);

	void ZeusBot(CUserCmd * cmd, C_BaseCombatWeapon * weapon, bool & bSendPacket);
	CUserCmd* CurrentCmd = nullptr;
	int FindBestEntity(CUserCmd * cmd, C_BaseCombatWeapon * weapon, Vector & hitpos, bool & bBacktrack);

	int LastRbotEnemyIndex = -1;
	std::vector<PointScanStruct> GetPointsForScan(C_BasePlayer* entity, int hitbox, mstudiohitboxset_t* hitset, matrix3x4_t matrix[MAXSTUDIOBONES], float pointscale);
};

extern RbotMatrixData matrixData[128];