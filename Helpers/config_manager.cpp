#include "config_manager.hpp"
#include "../options.hpp"
#include "../imgui/imgui.h"

void CConfig::SetupValue(int &value, int def, std::string category, std::string name)
{
	value = def;
	ints.push_back(new ConfigValue<int>(category, name, &value));
}

void CConfig::SetupValue(bool &value, bool def, std::string category, std::string name)
{
	value = def;
	bools.push_back(new ConfigValue<bool>(category, name, &value));
}

void CConfig::SetupValue(float &value, float def, std::string category, std::string name)
{
	value = def;
	floats.push_back(new ConfigValue<float>(category, name, &value));
}

void CConfig::SetupColor(float value[4], std::string name) {
	for (int i = 0; i < 4; i++)
		SetupValue(value[i], value[i], "Colors", name + "_" + std::to_string(i).c_str());
}

void CConfig::SetupRage()
{
	SetupValue(Vars.ragebot_enabled, false, "RAGEBOT", "enabled");
	SetupValue(Vars.ragebot_fov, 0, "RAGEBOT", "fov");
	SetupValue(Vars.ragebot_autozeus, false, "RAGEBOT", "autozeus");
	for (int type = WEAPON_GROUPS::PISTOLS; type <= WEAPON_GROUPS::SMG; type++)
	{
		SetupValue(Vars.ragebot_autoscope[type], false, "RAGEBOT", "autoscope_" + std::to_string(type));
		SetupValue(Vars.ragebot_hitchance[type], 0.f, "RAGEBOT", "hitchance" + std::to_string(type));
		SetupValue(Vars.ragebot_mindamage[type], 0.f, "RAGEBOT", "mindamage" + std::to_string(type));
		SetupValue(Vars.ragebot_baim_after_shots[type], 0, "RAGEBOT", "baim_after" + std::to_string(type));
		SetupValue(Vars.ragebot_selection[type], 0, "RAGEBOT", "targerselecion" + std::to_string(type));
		for (int i = 0; i < 8; i++) {
			SetupValue(Vars.ragebot_hitbox[i][type], false, "RAGEBOT", "hitscan_" + std::to_string(i) + std::to_string(type));
			SetupValue(Vars.ragebot_hitbox_multipoint_scale[i][type], 0.f, "RAGEBOT", "pointscale_" + std::to_string(i) + std::to_string(type));
		}
	}
	SetupValue(Vars.ragebot_autopistol, false, "RAGEBOT", "autopistol");
	SetupValue(Vars.ragebot_resolver, false, "RAGEBOT", "resolver");
	SetupValue(Vars.ragebot_fakelag_amt, 0, "RAGEBOT", "fakelag");
	SetupValue(Vars.ragebot_slowwalk_amt, 0, "RAGEBOT", "slowwalk");
	SetupValue(Vars.ragebot_slowwalk_key, 0, "RAGEBOT", "slowwalk_key");
	SetupValue(Vars.ragebot_antiaim_pitch, 0, "RAGEBOT", "antiaim_pitch");
	SetupValue(Vars.ragebot_antiaim_pitch_move, 0, "RAGEBOT", "antiaim_pitch_move");
	SetupValue(Vars.ragebot_antiaim_pitch_air, 0, "RAGEBOT", "antiaim_pitch_air");
	SetupValue(Vars.ragebot_antiaim_yaw, 0, "RAGEBOT", "antiaim_yaw");
	SetupValue(Vars.ragebot_antiaim_yaw_move, 0, "RAGEBOT", "antiaim_yaw_move");
	SetupValue(Vars.ragebot_antiaim_yaw_air, 0, "RAGEBOT", "antiaim_yaw_air");
	SetupValue(Vars.ragebot_antiaim_desync, false, "RAGEBOT", "desync");
	SetupValue(Vars.ragebot_antiaim_desynctype, false, "RAGEBOT", "desynctype");
	SetupValue(Vars.ragebot_antiaim_desyncrange, 0, "RAGEBOT", "desyncrange");

	for (int i = 0; i < 8; i++) {
		SetupValue(Vars.ragebot_fakelag_flags[i], false, "RAGEBOT", "fakelagflags");
	}

	SetupValue(Vars.ragebot_fakelag_type, 0, "RAGEBOT", "fakelagtype");
}

void CConfig::SetupSkinchanger()
{
	SetupValue(Vars.weapon[WEAPON_KNIFE].enabled, false, "Knife CT", "skin_enabled");
	SetupValue(Vars.weapon[WEAPON_KNIFE].seed, 0, "Knife CT", "skin_seed");
	SetupValue(Vars.weapon[WEAPON_KNIFE].stat_trak, 0, "Knife CT", "skin_stat_trak");
	SetupValue(Vars.weapon[WEAPON_KNIFE].wear, 0.001f, "Knife CT", "skin_wear");
	SetupValue(Vars.weapon[WEAPON_KNIFE].paint_kit_vector_index, 0, "Knife CT", "skin_paintkit_selected_index");
	SetupValue(Vars.weapon[WEAPON_KNIFE].definition_override_vector_index, 0, "Knife CT", "skin_model_selected_index");

	SetupValue(Vars.weapon[WEAPON_KNIFE_T].enabled, false, "Knife TT", "skin_enabled");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].seed, 0, "Knife TT", "skin_seed");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].stat_trak, 0, "Knife TT", "skin_stat_trak");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].wear, 0.001f, "Knife TT", "skin_wear");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].paint_kit_vector_index, 0, "Knife TT", "skin_paintkit_selected_index");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].definition_override_vector_index, 0, "Knife TT", "skin_model_selected_index");

	SetupValue(Vars.weapon[5028].enabled, false, "Glove", "skin_enabled");
	SetupValue(Vars.weapon[5028].wear, 0.001f, "Glove", "skin_wear");
	SetupValue(Vars.weapon[5028].definition_override_vector_index, 0, "Glove", "skin_model_selected_index");
	SetupValue(Vars.weapon[5028].paint_kit_vector_index, 0, "Glove", "skin_paintkit_selected_index");

}

void CConfig::SetupMisc()
{
	SetupValue(Vars.misc_bhop, false, "MISC", "bunnyhop");
	SetupValue(Vars.misc_autostrafe, false, "MISC", "autostrafe");
	SetupValue(Vars.misc_clantag, false, "MISC", "clantag");
	SetupValue(Vars.misc_clantagoptions, 0, "MISC", "clantagops");
	SetupValue(Vars.misc_autoaccept, false, "MISC", "autoaccept");
	SetupValue(Vars.misc_fastduck, false, "MISC", "fastduck");
	SetupValue(Vars.misc_fakeduck, false, "MISC", "fakeduck");
	SetupValue(Vars.misc_fakeduck_key, 0, "MISC", "fakeduck_key");

	SetupValue(Vars.misc_spectlist, false, "MISC", "spectatorlist");
	SetupValue(Vars.misc_overridefov, 0, "MISC", "overridefov");
	SetupValue(Vars.misc_viewmodelfov, 0, "MISC", "viewmodelfov");
}

void CConfig::SetupVisuals()
{
	SetupValue(Vars.visuals_spreadxair, false, "VISUALS", "spreadxair");
	SetupColor(Vars.visuals_spreadxair_color, "spread_color");

	SetupValue(Vars.visuals_snapline, false, "VISUALS", "snapline");
	SetupColor(Vars.visuals_snaplines, "snaplines_color");

	SetupValue(Vars.visuals_norecoil, false, "VISUALS", "norecoil");

	SetupValue(Vars.visuals_nosmoke, false, "VISUALS", "nosmoke");
	SetupValue(Vars.visuals_noscopezoom, false, "VISUALS", "noscopezoom");

	SetupValue(Vars.visuals_radar, false, "VISUALS", "radar");

	SetupValue(Vars.visuals_noscopeborder, false, "VISUALS", "noscopeborders");
	SetupValue(Vars.visuals_thirdperson, false, "VISUALS", "thirdperson");
	SetupValue(Vars.visuals_thirdperson_key, false, "VISUALS", "thirdperson_key");

	SetupValue(Vars.misc_overridefov_inscope, false, "VISUALS", "velocity");
	SetupValue(Vars.visuals_watermark, true, "VISUALS", "watermark");
	SetupColor(Vars.color_counter, "color_watermark");

	SetupValue(Vars.visuals_noflash, false, "VISUALS", "no_flash");

	SetupValue(Vars.esp_enabled, false, "ESP", "enabled");
	SetupValue(Vars.esp_ignore_team, false, "ESP", "ignore_team");
	SetupValue(Vars.esp_visible_only, false, "ESP", "visible_only");
	SetupValue(Vars.esp_player_boxes, false, "ESP", "boxes");
	SetupValue(Vars.esp_player_names, false, "ESP", "names");
	SetupValue(Vars.esp_player_weapons, false, "ESP", "weapon");
	SetupValue(Vars.esp_player_health, false, "ESP", "health");
	SetupValue(Vars.esp_player_armour, false, "ESP", "armor");

	SetupValue(Vars.chams_player_enabled, false, "CHAMS", "enabled");
	SetupValue(Vars.chams_player_ignore_team, false, "CHAMS", "ignore_team");
	SetupValue(Vars.chams_player_visible_only, false, "CHAMS", "visible_only");

	SetupValue(Vars.glow_enabled, false, "GLOW", "enabled");
	SetupValue(Vars.glow_ignore_team, false, "GLOW", "ignore_team");
	SetupValue(Vars.glow_visible_only, false, "GLOW", "visible_only");

	SetupValue(Vars.esp_drop_enable, false, "Item ESP", "drop_enable");
	SetupValue(Vars.esp_drop_distance, 0, "Item ESP", "drop_distance");

	SetupValue(Vars.esp_dropped_weapons, false, "Item ESP", "dropped_weapons");
	SetupValue(Vars.esp_planted_c4, false, "Item ESP", "planted_c4");
	SetupValue(Vars.esp_case_pistol, false, "Item ESP", "case_pistol");
	SetupValue(Vars.esp_case_light_weapon, false, "Item ESP", "light_weapon");
	SetupValue(Vars.esp_case_heavy_weapon, false, "Item ESP", "heavy_weapon");
	SetupValue(Vars.esp_case_explosive, false, "Item ESP", "case_explosive");
	SetupValue(Vars.esp_case_tools, false, "Item ESP", "case_tools");
	SetupValue(Vars.esp_random, false, "Item ESP", "random");
	SetupValue(Vars.esp_dz_armor_helmet, false, "Item ESP", "dz_armor_helmet");
	SetupValue(Vars.esp_dz_helmet, false, "Item ESP", "dz_helmet");
	SetupValue(Vars.esp_dz_armor, false, "Item ESP", "dz_armor");
	SetupValue(Vars.esp_upgrade_tablet, false, "Item ESP", "upgrade_tablet");
	SetupValue(Vars.esp_briefcase, false, "Item ESP", "briefcase");
	SetupValue(Vars.esp_parachutepack, false, "Item ESP", "parachutepack");
	SetupValue(Vars.esp_dufflebag, false, "Item ESP", "dufflebag");
	SetupValue(Vars.esp_ammobox, false, "Item ESP", "ammobox");

	SetupValue(Vars.visuals_nightmode, false, "VISUALS", "nightmode");
}

void CConfig::SetupColors()
{
	SetupColor(Vars.color_esp_enemy_visible, "color_esp_enemy_visible");
	SetupColor(Vars.color_esp_enemy_hidden, "color_esp_enemy_hidden");
	SetupColor(Vars.color_esp_team_visible, "color_esp_team_visible");
	SetupColor(Vars.color_esp_team_hidden, "color_esp_team_hidden");

	SetupColor(Vars.color_chams_enemy_visible, "color_chams_enemy_visible");
	SetupColor(Vars.color_chams_enemy_hidden, "color_chams_enemy_hidden");
	SetupColor(Vars.color_chams_team_visible, "color_chams_team_visible");
	SetupColor(Vars.color_chams_team_hidden, "color_chams_team_hidden");

	SetupColor(Vars.esp_planted_c4_clr, "color_planted_c4");
	SetupColor(Vars.esp_dropped_clr, "color_dropped_weapon");
}

void CConfig::Initialize()
{
	CConfig::SetupRage();
	CConfig::SetupSkinchanger();
	CConfig::SetupMisc();
	CConfig::SetupVisuals();
	CConfig::SetupColors();
}

void CConfig::Save(std::string szIniFile)
{
	std::string folder, file;
	folder = XorStr("C:/Sketu.xyz/");

	file = folder + szIniFile + XorStr(".sketu");

	CreateDirectoryA(folder.c_str(), NULL);

	for (auto value : ints)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : floats)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : bools)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), *value->value ? "1" : "0", file.c_str());
}

void CConfig::Load(std::string szIniFile)
{
	std::string folder, file;

	folder = XorStr("C:/Sketu.xyz/");

	file = folder + szIniFile + XorStr(".sketu");

	CreateDirectoryA(folder.c_str(), NULL);

	char value_l[32] = { '\0' };

	for (auto value : ints)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atoi(value_l);
	}

	for (auto value : floats)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = (float)atof(value_l);
	}

	for (auto value : bools)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = !strcmp(value_l, "1");
	}
}









































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































