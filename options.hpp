#pragma once

#include <string>
#include <vector>
#include <Windows.h>

#include <map>
#include "Valve_SDK/Misc/Color.hpp"
#include "Features/skinchanger/item_definitions.hpp"
#include "ImGui/imgui.h"

#define OPTION(type, var, val) type var = val

enum WEAPON_GROUPS {
	PISTOLS,
	RIFLES,
	SHOTGUNS,
	SCOUT,
	AUTO,
	AWP,
	SMG,
	UNKNOWN
};

class CVariables
{
public:
	float barcolour[4] = { 1.f, 0.f, 1.f, 1.f };

	struct Colour4
	{
		ImColor x, y, h, w;

		Colour4() { x = y = h = w = 0.0f; }
		Colour4(ImColor _x, ImColor _y, ImColor _h, ImColor _w) { x = _x; y = _y; h = _h; w = _w; }
	};
	Colour4 barColor1;
	Colour4 barColor2;

	float color_menu_shit[4] = { 1.f, 1.f, 1.f, 1.f };

	bool ragebot_antiaim_slidewalk = false;

	int ragebot_antiaim_stand_real_add = 0;
	int ragebot_antiaim_move_real_add = 0;
	int ragebot_antiaim_air_real_add = 0;
	float ragebot_antiaim_stand_real_add_range = 0;
	float ragebot_antiaim_move_real_add_range = 0;
	float ragebot_antiaim_air_real_add_range = 0;

	float ragebot_antiaim_spinspeed = 0.f;

	bool ragebot_antiaim_fakeduck = false;
	int ragebot_antiaim_fakeduck_key = 0;

	bool ragebot_resolver = false;

	bool ragebot_autopistol = false;

	bool ragebot_antiaim_enable = false;

	int ragebot_antiaim_pitch = 0;
	int ragebot_antiaim_pitch_move = 0;
	int ragebot_antiaim_pitch_air = 0;
	int ragebot_antiaim_yaw = 0;
	int ragebot_antiaim_yaw_move = 0;
	int ragebot_antiaim_yaw_air = 0;
	float ragebot_fakelag_amt = 1;
	bool ragebot_fakelag_flags[3] = { false, false, false };
	int ragebot_fakelag_type = 1;
	int ragebot_fakelag_amt_move = 0;
	int ragebot_fakelag_amt_air = 0;
	int ragebot_fakelag_amt_air_mode = 0;
	int ragebot_fakelag_amt_move_mode = 0;
	bool ragebot_antiaim_desync = false;
	int ragebot_slowwalk_amt = 0;
	int ragebot_slowwalk_key = -1;

	int ragebot_antiaim_manual_left = -1;
	int ragebot_antiaim_manual_right = -1;
	int ragebot_antiaim_manual_back = -1;

	int ragebot_antiaim_desyncrange = 0;

	int ragebot_antiaim_desynctype = 0;
	
	bool ragebot_enabled = false;
	bool ragebot_backtrack = false;
	int ragebot_fov = 0;
	bool ragebot_autozeus = false;

	bool ragebot_hitbox[8][9] = { false, false, false, false, false, false, false, false };
	float ragebot_hitbox_multipoint_scale[8][9] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	int ragebot_selection[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	bool ragebot_autoscope[9] = { false, false, false, false, false, false, false, false };
	float ragebot_mindamage[9] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float ragebot_hitchance[9] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	int ragebot_baim_after_shots[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	struct weapon_settings {
		char name[32] = "Default";
		bool enabled = false;
		int definition_vector_index = 0;
		int definition_index = 1;
		int paint_kit_vector_index = 0;
		int paint_kit_index = 0;
		int definition_override_vector_index = 0;
		int definition_override_index = 0;
		int seed = 0;
		int stat_trak = 0;
		float wear = 0;
		char custom_name[32] = "";
	};

	std::map<int, weapon_settings> weapon = {};

	struct {
		std::map<int, weapon_settings> m_items;
		std::unordered_map<std::string, std::string> m_icon_overrides;
		auto get_icon_override(const std::string original) const -> const char*
		{
			return m_icon_overrides.count(original) ? m_icon_overrides.at(original).data() : nullptr;
		}
	} skins;

	bool visuals_spreadxair = false;
	float visuals_spreadxair_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool visuals_snapline = false;
	float visuals_snaplines[4] = { 1.f, 1.f, 1.f, 1.f };
	float esp_dropped_clr[4] = { 1.f, 1.f, 1.f, 1.f };
	float esp_planted_c4_clr[4] = { 1.f, 1.f, 1.f, 1.f };
	bool visuals_watermark = true;
	bool visuals_ind = false;
	float color_counter[4] = { 0.f, 0.f, 0.f, 0.180f };
	bool visuals_crosshair = true;
	bool visuals_norecoil = false;
	bool visuals_noscopezoom = false;
	bool visuals_nightmode = false;
	bool visuals_noflash = false;

	bool visuals_fullbright = false;

	bool visuals_bullettracer = false;
	float visuals_bullettracer_clr[4] = { 1.f, 1.f, 1.f, 1.f };

	bool misc_overridefov_inscope = false;
	bool visuals_thirdperson = false;
	int visuals_thirdperson_key = -1;

	bool visuals_nosmoke = false;

	bool visuals_noscopeborder = false;
	int visuals_noscopeborderoptions = 0;

	bool esp_enabled = false;
	bool esp_ignore_team = false;
	bool esp_visible_only = false;
	bool esp_player_boxes = false;
	bool esp_player_names = false;
	bool esp_player_health = false;
	int esp_player_health_type = 0;
	bool esp_player_armour = false;
	int esp_player_armour_type = 0;
	bool esp_player_weapons = false;
	bool esp_player_snaplines = false;

	bool esp_drop_enable = false;
	int esp_drop_distance = 3500;

	bool visuals_radar = false;

	bool esp_dropped_weapons = false;
	bool esp_defuse_kit = false;
	bool esp_planted_c4 = false;

	bool esp_case_pistol = false;
	bool esp_case_light_weapon = false;
	bool esp_case_heavy_weapon = false;
	bool esp_case_explosive = false;
	bool esp_case_tools = false;
	bool esp_random = false;
	bool esp_dz_armor_helmet = false;
	bool esp_dz_helmet = false;
	bool esp_dz_armor = false;
	bool esp_upgrade_tablet = false;
	bool esp_briefcase = false;
	bool esp_parachutepack = false;
	bool esp_dufflebag = false;
	bool esp_ammobox = false;

	bool glow_enabled = false;
	bool glow_ignore_team = false;
	bool glow_visible_only = false;
	bool chams_player_enabled = false;
	bool chams_player_ignore_team = false;
	bool chams_player_visible_only = false;

	bool chams_player_local = false;
	float color_chams_local[4] = { 1.f, 1.f, 1.f, 1.f };

	bool misc_latency_enable = false;
	int misc_latency_amt = 0;
	bool legit_backtracking = false;
	bool misc_antikick = false;
	bool misc_autoaccept = false;
	bool misc_spectlist = false;
	bool misc_clantag = false;
	int misc_clantagoptions = 0;
	bool visuals_bulletimpacts = false;

	int misc_info = 0;

	char misc_clantagtype = 0;

	bool misc_fastduck = false;
	bool misc_fakeduck = false;
	int misc_fakeduck_key = -1;
	bool misc_bhop = false;
	bool misc_autostrafe = false;
	int misc_overridefov = 0;
	int misc_viewmodelfov = 0;
	bool misc_thirdperson = false;
	bool misc_showranks = true;
	float misc_thirdperson_dist = 50.f;
	float mat_ambient_light_r = 0.0f;
	float mat_ambient_light_g = 0.0f;
	float mat_ambient_light_b = 0.0f;

	bool misc_buybot = false;
	int misc_buybotoptions = 0;
	int misc_buybotoptions_pistol = 0;
	bool misc_buybotoptions_grenades[8] = { false, false, false, false, false, false, false, false };

	bool misc_chatspam = false;

	bool misc_killsay = false;

	bool misc_namespam = false;

	bool radio_paused = true;
	int radio_selected = 0;

	bool misc_killfeed = false;

	bool misc_autobuy = false;
	int misc_autobuyprimary = 0;
	int misc_autobuysecondary = 0;
	bool misc_autobuygrenades[8] = { false, false, false, false, false, false, false, false };

	bool misc_viewmodel_offset = false;
	int misc_viewmodeloffset_x = 2;
	int misc_viewmodeloffset_y = -2;
	int misc_viewmodeloffset_z = -2;

	bool misc_hitsound = false;
	int misc_hitsoundtype = 0;

	float color_esp_enemy_visible[4] = { 1.f, 0.f, 0.f, 1.f };
	float color_esp_enemy_hidden[4] = { 0.f, 0.f, 0.f, 1.f };
	float color_esp_team_visible[4] = { 1.f, 0.f, 0.f, 1.f };
	float color_esp_team_hidden [4] = { 0.f, 0.f, 0.f, 1.f };

	float color_chams_enemy_visible[4]  = { 1.f, 0.f, 0.f, 1.f };
	float color_chams_enemy_hidden[4] = { 0.f, 0.f, 0.f, 1.f };
	float color_chams_team_visible[4]  = { 1.f, 0.f, 0.f, 1.f };
	float color_chams_team_hidden [4] = { 0.f, 0.f, 0.f, 1.f };
};

extern CVariables Vars;
extern CVariables::weapon_settings* WeaponSettings;
extern std::string ActiveWeaponName;

namespace Globals
{
	extern QAngle LastAngle;
	extern bool ThirdPersponToggle;
	extern bool Unload;
	extern bool MenuOpened;
	extern bool PlayerListOpened;
	extern bool RadioOpened;
	extern bool WeaponTabValid;
	extern Vector aim_point;
}