#pragma once

#include "../../Helpers/singleton.hpp"

#include "../../Helpers/draw_manager.hpp"
#include "../../Helpers/math.hpp"
#include "../../Valve_SDK/csgostructs.hpp"

class CVisuals : public Singleton<CVisuals>
{
public:
	void RenderESP();
	void Velocity();
	void ThirdPerson();
	void DrawSoundBeam(Vector center);
	void NoSmoke();
	bool ValidPlayer(C_BasePlayer * player, bool count_step);
	void DrawBulletImpacts();
	void DrawSkeleton(C_BasePlayer * entity, Color color, const matrix3x4_t * bone_matrix);
	void RenderWeapon(C_BaseCombatWeapon * ent);
	void DrawGrenades(C_BaseEntity * ent);
	void RenderWeapon();
	void Radar();
	void RenderDrop();
	void RenderDefuseKit(C_BaseEntity * ent);
	void RenderPlantedC4(C_BaseEntity * ent);
	bool Begin(C_BasePlayer * pl);
	void RenderBox();
	void HealthBar();
	void HealthBar2();
	void ArmorBar2();
	void ArmorBar();
	void RenderName();
	void RenderSpreadXair();
	void DrawAimbotFov();
	void SnapLine();
	void Crosshair();
	void NoScopeBorder();
	void RenderParticles();
	void PaintTraverse();
	void Watermark();
	void Ind();
};

extern bool hardDisable;

class CHudModulation : public Singleton<CHudModulation>
{
public:
	void FrameStageNotify(ClientFrameStage_t stage);
	void ToggleVGuiModulation(bool enabled);
};

void obrabotka();