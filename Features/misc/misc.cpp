#include "misc.hpp"
#include "../../options.hpp"
#include "../../Valve_SDK/csgostructs.hpp"
#include "../../ImGui/imgui.h"
#include "../../hooks.hpp"
#include "../../Helpers/input.hpp"
#include "../../Helpers/math.hpp"
#include <time.h>

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

void CMisc::SetNorecoilAngles(ClientFrameStage_t stage, QAngle* aim_punch, QAngle* view_punch, QAngle old_aim_punch, QAngle old_view_punch)
{
	if (stage != FRAME_RENDER_START)
		return;

	if (!Globals::LocalPlayer)
		return;

	if (!Globals::LocalPlayer->IsAlive())
		return;

	if (Interfaces::Input->m_fCameraInThirdPerson)
		return;

	if (true)
	{
		aim_punch = &old_aim_punch;
		view_punch = &old_view_punch;
	}
}

void CMisc::FakeDuck(CUserCmd* cmd, bool& bSendPacket)
{
	if (!Vars.misc_fakeduck)
		return;

	if (!Utils::IsInGame())
		return;

	if (GetAsyncKeyState(Vars.misc_fakeduck_key))
	{
		if (Vars.misc_fakeduck)
		{
			int fakelag_limit = Vars.ragebot_fakelag_amt >= 14 ? 14 : Vars.ragebot_fakelag_amt;
			int choked_goal = fakelag_limit / 2;
			bool crouch = Interfaces::ClientState->chokedcommands >= choked_goal;

			if (G::LocalPlayer->m_fFlags() & FL_ONGROUND)
			{
				cmd->buttons |= IN_BULLRUSH;

				if (crouch)
					cmd->buttons |= IN_DUCK;
				else
				{
					cmd->buttons &= ~IN_DUCK;
					bSendPacket = true;
				}
			}
		}
	}
}

void CMisc::NoVisualRecoil(ClientFrameStage_t stage, QAngle* aim_punch, QAngle* view_punch, QAngle old_aim_punch, QAngle old_view_punch)
{
	if (stage != FRAME_RENDER_START)
		return;

	if (!Globals::LocalPlayer)
		return;

	if (!Globals::LocalPlayer->IsAlive())
		return;

	if (Interfaces::Input->m_fCameraInThirdPerson)
		return;

	if (view_punch && aim_punch/* && VisualElements.Visual_Misc_NoRecoil->Checked*/)
	{
		old_view_punch = *view_punch;
		old_aim_punch = *aim_punch;

		view_punch->Init();
		aim_punch->Init();
	}
}

void CMisc::SetThirdpersonAngles(ClientFrameStage_t stage)
{
	if (stage != FRAME_RENDER_START)
		return;

	if (Utils::IsInGame() && Globals::LocalPlayer)
	{
		if (Globals::LocalPlayer->IsAlive() && Interfaces::Input->m_fCameraInThirdPerson)
			Globals::LocalPlayer->SetVAngles(Globals::LastAngle);
	}
}

void CMisc::FakeLag(CUserCmd* cmd, bool& bSendPacket)
{
	if (!Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive())
		return;

	auto weapon = Globals::LocalPlayer->m_hActiveWeapon();

	static int ticks = 0;
	int choke;
	bool fakelag = false;
	bool should_fakelag = false;

	switch (Vars.ragebot_fakelag_type) 
	{
	case 1:
		choke = Vars.ragebot_fakelag_amt;
		break;
	}

	if (Interfaces::Engine->IsVoiceRecording())
		choke = 1;

	if (weapon->IsGrenade())
		choke = 1;

	if (Vars.ragebot_fakelag_flags[0]) 
	{
		if (Globals::LocalPlayer->m_fFlags() & FL_ONGROUND && Globals::LocalPlayer->m_vecVelocity().Length2D() < 0.1f)
			should_fakelag = true;
	}

	if (Vars.ragebot_fakelag_flags[1])
	{
		if (Globals::LocalPlayer->m_vecVelocity().Length() > 0.1f && Globals::LocalPlayer->m_fFlags() & FL_ONGROUND)
			should_fakelag = true;
	}

	if (Vars.ragebot_fakelag_flags[2]) 
	{
		if (!(Globals::LocalPlayer->m_fFlags() & FL_ONGROUND))
			should_fakelag = true;
	}

	if (Vars.ragebot_fakelag_flags[3])
	{
		if (GetAsyncKeyState(VK_SHIFT))
			choke = 14, should_fakelag = true;
	}

	if (should_fakelag)
	{
		if (ticks > choke)
		{
			ticks = 0;
			bSendPacket = true;
		}
		else 
		{
			bSendPacket = false;
			ticks++;
		}

		fakelag = true;
	}
	else
		fakelag = false;
}

void CMisc::SlowWalk(CUserCmd *cmd)
{
	if (Vars.ragebot_slowwalk_amt <= 0 || !GetAsyncKeyState(Vars.ragebot_slowwalk_key))
		return;

	auto weapon_handle = Globals::LocalPlayer->m_hActiveWeapon();

	if (!weapon_handle)
		return;

	float amount = 0.0034f * Vars.ragebot_slowwalk_amt/*options.misc.slow_walk_amount*/; //max 100

	Vector velocity = Globals::LocalPlayer->m_vecVelocity();
	QAngle direction;

	Math::VectorAngles(velocity, direction);

	float speed = velocity.Length2D();

	direction.yaw = cmd->viewangles.yaw - direction.yaw;

	Vector forward;

	Math::AngleVectors(direction, forward);

	Vector source = forward * -speed;

	if (speed >= (weapon_handle->GetCSWeaponData()->flMaxPlayerSpeed * amount))
	{
		cmd->forwardmove = source.x;
		cmd->sidemove = source.y;

	}
}

auto set_ct(const char* tag)
{
	using set_clan_tag_fn = int(__fastcall*)(const char*, const char*);
	static auto set_clan_tag = reinterpret_cast<set_clan_tag_fn>(Utils::PatternScan(GetModuleHandleA("engine.dll"), "53 56 57 8B DA 8B F9 FF 15"));
}

void CMisc::ClanTag()
{
	if (!Utils::IsInGame())
		return;

	if (!Vars.misc_clantag)
		return;

	static int counter = 0;
	static int motion = 0;
	int ServerTime = (float)g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick * 3.5;

	if (counter % 48 == 0)
		motion++;

	int value = ServerTime % 31;

	switch (value)
	{
	case 0:  Utils::SetClantag(XorStr("S")); break;
	case 1:  Utils::SetClantag(XorStr("S$")); break;
	case 2:  Utils::SetClantag(XorStr("SK")); break;
	case 3:  Utils::SetClantag(XorStr("SK$")); break;
	case 4:  Utils::SetClantag(XorStr("SKE")); break;
	case 5:  Utils::SetClantag(XorStr("SKE$")); break;
	case 6:  Utils::SetClantag(XorStr("SKET")); break;
	case 7:  Utils::SetClantag(XorStr("SKET$")); break;
	case 8:  Utils::SetClantag(XorStr("SKETU")); break;
	case 9:  Utils::SetClantag(XorStr("SKETU$")); break;
	case 11: Utils::SetClantag(XorStr("SKETU.")); break;
	case 12: Utils::SetClantag(XorStr("SKETU.$")); break;
	case 13: Utils::SetClantag(XorStr("SKETU.X")); break;
	case 14: Utils::SetClantag(XorStr("SKETU.X$")); break;
	case 15: Utils::SetClantag(XorStr("SKETU.XY")); break;
	case 16: Utils::SetClantag(XorStr("SKETU.XY$")); break;
	case 17: Utils::SetClantag(XorStr("SKETU.XYZ")); break;
	case 18: Utils::SetClantag(XorStr("SKETU.XY$")); break;
	case 19: Utils::SetClantag(XorStr("SKETU.X$")); break;
	case 20: Utils::SetClantag(XorStr("SKETU.X")); break;
	case 21: Utils::SetClantag(XorStr("SKETU.$")); break;
	case 22: Utils::SetClantag(XorStr("SKETU.")); break;
	case 23: Utils::SetClantag(XorStr("SKETU$")); break;
	case 24: Utils::SetClantag(XorStr("SKETU")); break;
	case 25: Utils::SetClantag(XorStr("SKET$")); break;
	case 26: Utils::SetClantag(XorStr("SKET")); break;
	case 27: Utils::SetClantag(XorStr("SKE$")); break;
	case 28: Utils::SetClantag(XorStr("SKE")); break;
	case 29: Utils::SetClantag(XorStr("SK$")); break;
	case 30: Utils::SetClantag(XorStr("SK")); break;
	case 31: Utils::SetClantag(XorStr("S$")); break;
	case 32: Utils::SetClantag(XorStr("S")); break;
	}
	counter++;
}

template<class T, class U>
T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}

void CMisc::AutoStrafe(CUserCmd* cmd)
{
	if (!Vars.misc_autostrafe)
		return;

	if (!Utils::IsInGame())
		return;

	if (!Globals::LocalPlayer)
		return;

	if (!Globals::LocalPlayer->IsAlive()) 
		return;

	if (Globals::LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	if (Globals::LocalPlayer->m_nMoveType() == MOVETYPE_LADDER)
		return;

	if (!(Globals::LocalPlayer->m_fFlags() & FL_ONGROUND))
	{
		if (cmd->mousedx > 1 || cmd->mousedx < -1)
		{
			cmd->sidemove = clamp(cmd->mousedx < 0.f ? -400.f : 400.f, -400, 400);
		}
		else
		{
			if (Globals::LocalPlayer->m_vecVelocity().Length2D() == 0 || Globals::LocalPlayer->m_vecVelocity().Length2D() == NAN || Globals::LocalPlayer->m_vecVelocity().Length2D() == INFINITE)
			{
				cmd->forwardmove = 400;
				return;
			}

			cmd->forwardmove = clamp(5850.f / Globals::LocalPlayer->m_vecVelocity().Length2D(), -400, 400);
			
			if (cmd->forwardmove < -400 || cmd->forwardmove > 400)
				cmd->forwardmove = 0;

			cmd->sidemove = clamp((cmd->command_number % 2) == 0 ? -400.f : 400.f, -400, 400);
			
			if (cmd->sidemove < -400 || cmd->sidemove > 400)
				cmd->sidemove = 0;
		}
	}
}

void CMisc::ViewmodelChanger()
{
	if (!Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive())
		return;

	if (!Utils::IsInGame())
		return;

	static int vx, vy, vz, b1g;
	static ConVar* view_x = Interfaces::Convar->FindVar("viewmodel_offset_x");
	static ConVar* view_y = Interfaces::Convar->FindVar("viewmodel_offset_y");
	static ConVar* view_z = Interfaces::Convar->FindVar("viewmodel_offset_z");

	static ConVar* bob = Interfaces::Convar->FindVar("cl_bobcycle");
}

void CMisc::NoView(CViewSetup* vsView)
{
	if (!Globals::LocalPlayer)
		return;

	if (!Globals::LocalPlayer->IsAlive())
		return;

	if (!Utils::IsInGame())
		return;

	if (!Globals::LocalPlayer->m_bIsScoped())
	{
		vsView->fov = 90 + Vars.misc_overridefov;
	}
	else
	{
		if (Vars.visuals_noscopezoom)
			vsView->fov = 90 + Vars.misc_overridefov;
		else
		{

		}
	}
}

void CMisc::OverrideView(CViewSetup* vsView)
{
	if (!Globals::LocalPlayer)
		return;

	if (Globals::LocalPlayer->m_bIsScoped() && !Vars.misc_overridefov_inscope)
		return;

	if (Vars.misc_overridefov != 0)
		vsView->fov = (float)Vars.misc_overridefov;
}

void CMisc::FastDuck(CUserCmd* cmd)
{
	if (Vars.misc_fastduck)
		cmd->buttons |= IN_BULLRUSH;
}

void CMisc::Bhop(CUserCmd* cmd)
{
	if (!Vars.misc_bhop)
		return;

	static bool jumped_last_tick = false;
	static bool should_fake_jump = false;

	if (!jumped_last_tick && should_fake_jump) 
	{
		should_fake_jump = false;
		cmd->buttons |= IN_JUMP;
	}
	else if (cmd->buttons & IN_JUMP)
	{
		if (Globals::LocalPlayer->m_fFlags() & FL_ONGROUND) 
		{
			jumped_last_tick = true;
			should_fake_jump = true;
		}
		else 
		{
			cmd->buttons &= ~IN_JUMP;
			jumped_last_tick = false;
		}
	}
	else
	{
		jumped_last_tick = false;
		should_fake_jump = false;
	}
}

void CMisc::AutoAccept(const char* pSoundEntry)
{
	if (!Vars.misc_autoaccept)
		return;

	if (!strcmp(pSoundEntry, XorStr("UIPanorama.popup_accept_match_beep"))) 
	{
		static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA(XorStr("client_panorama.dll")), XorStr("55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12")));

		if (fnAccept) {

			fnAccept("");

			FLASHWINFO fi;
			fi.cbSize = sizeof(FLASHWINFO);
			fi.hwnd = InputSys::Get().GetMainWindow();
			fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
			fi.uCount = 0;
			fi.dwTimeout = 0;
			FlashWindowEx(&fi);
		}
	}
}

































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































class pdkwpzr {
public:
	string iihjtdfjvj;
	double dmqlndvxdmhvngp;
	pdkwpzr();
	double hknarldhltbshxenaz(bool xwhyiq, string tlknyn, double vcbocugxpz, bool vvdifjwum, double cdvxfcx, int jyspqnkmy, int vewdyixcvyj, int zjeav, bool raabsxanpbhablx);
	bool ngtbwgyrqib(bool mffrtmldqrjzg, int zkppamjigyzdp, int uegdypa, bool oylcil, bool vhutq, int qeiitsy);
	string atyzthumkrwvskesiysdukjm(double sieqdzdjhpzfmf, string kevsthkeskkvtx, int juwkndscdycrmf, bool ghnowexvuwv, double frnphxgdpnwfv, double mmwxlsyklufgoo, double knpamhsujm, bool inflmmcgdks);
	void kssfgygqdhdchxlexip(int lkumcspxrkjazdm, string hegtle, bool ltsta, int akjiur, string tattikqryz, double kqutkzikifexdjr, int bpgetc, bool xylnwe, double xbrecyucrsjngzm);
	bool almnnrcyhlrgafxhtl(bool qvhkhddt);

protected:
	bool ttdrdhgldjn;
	double klrdnmrr;

	double uefltcaxvdbydplfbtrbi(double zoupeinmxqfuyfk, bool hfbazdfvlrb);
	double murlcsxjpwbyqkwgvmckztp(bool hxqgq, int tqlfpdpxfpfvfs, int htdyjca, string oaoebaegybzqsdj, string ifxyhblfvfcvo, int fababoq);
	string mgbixxtvhvjresbwoyfrlytkr(bool nqvusiwuz, int bcbipsew);
	void daumqsipbnubfkbvug(double oizxn, string mrtiolclxfi, int axvdccpnbrzem, double ufcqasucofaj);

private:
	string wcxyoumgeindbmz;
	string efuazlwuptlcgw;
	string hxulvhyuk;
	bool rgfxf;
	int nzqrzhidmty;

	double awapblvprhn();
	string dmqiblusyzcbaaqubqhbaxqcq(int wpcmr, double zosvhjeb, double lrhxghciut, string sfnqx, string raauica, bool kxfpom, double nnvcdmjilbyjl, bool bnjkudqkzlprtmj);
	void fsvhnorepfmrojtl(double ytkiekmmzlnpswk, int vohiyftjpbvxi);
	double jkfeutigmkzroyxgtrtngo(int djqaotzkivxwo, int rtstogae);
	bool kxazrojgetbfrfgbcwifrrsb(double zwsoqgdcvea, bool arvuuy, double bwwzgyqjcruvak, bool hzvaytqunn, bool pewyrttxlplf, bool xkoeigogmphu, bool zjeiapvtpsgt);
	double owxqmvsgfkcvxxrwfvrjuh(bool yuxxkkskslnyzij, bool lzswp, string ccunlajiy, string ajxrq, bool psmylsueya, string imthaqhvj, string aibnsvtyblbs, double nrgig, int ergemedmvhs);

};



double pdkwpzr::awapblvprhn() {
	string ykowqfzufgca = "wvqsephh";
	int kovkiehbp = 1056;
	int huyeyaaljllxyk = 466;
	if (466 != 466) {
		int pycp;
		for (pycp = 79; pycp > 0; pycp--) {
			continue;
		}
	}
	return 97785;
}

string pdkwpzr::dmqiblusyzcbaaqubqhbaxqcq(int wpcmr, double zosvhjeb, double lrhxghciut, string sfnqx, string raauica, bool kxfpom, double nnvcdmjilbyjl, bool bnjkudqkzlprtmj) {
	bool uvmmnko = false;
	string elcebtasv = "lzwqauaomiowibqextphndhkmlpusbsnuckeotbmpwoquvfqyxcmqgbdjlxgahtupnggwodqqvnxfgqoampkwfdqkavw";
	if (false != false) {
		int fznq;
		for (fznq = 59; fznq > 0; fznq--) {
			continue;
		}
	}
	return string("");
}

void pdkwpzr::fsvhnorepfmrojtl(double ytkiekmmzlnpswk, int vohiyftjpbvxi) {

}

double pdkwpzr::jkfeutigmkzroyxgtrtngo(int djqaotzkivxwo, int rtstogae) {
	bool lilvvjxik = true;
	int fffju = 6276;
	int kegnjhngsgc = 64;
	bool vjhutsca = true;
	if (6276 == 6276) {
		int bloyitquoe;
		for (bloyitquoe = 87; bloyitquoe > 0; bloyitquoe--) {
			continue;
		}
	}
	return 57247;
}

bool pdkwpzr::kxazrojgetbfrfgbcwifrrsb(double zwsoqgdcvea, bool arvuuy, double bwwzgyqjcruvak, bool hzvaytqunn, bool pewyrttxlplf, bool xkoeigogmphu, bool zjeiapvtpsgt) {
	int maadckbvxxeiuzi = 2566;
	if (2566 != 2566) {
		int wwqt;
		for (wwqt = 25; wwqt > 0; wwqt--) {
			continue;
		}
	}
	if (2566 != 2566) {
		int bth;
		for (bth = 1; bth > 0; bth--) {
			continue;
		}
	}
	if (2566 == 2566) {
		int yu;
		for (yu = 97; yu > 0; yu--) {
			continue;
		}
	}
	if (2566 == 2566) {
		int smof;
		for (smof = 68; smof > 0; smof--) {
			continue;
		}
	}
	if (2566 != 2566) {
		int hxxsec;
		for (hxxsec = 50; hxxsec > 0; hxxsec--) {
			continue;
		}
	}
	return false;
}

double pdkwpzr::owxqmvsgfkcvxxrwfvrjuh(bool yuxxkkskslnyzij, bool lzswp, string ccunlajiy, string ajxrq, bool psmylsueya, string imthaqhvj, string aibnsvtyblbs, double nrgig, int ergemedmvhs) {
	double iyaxouku = 39131;
	bool bbyoqowwsuydszv = false;
	double fzzflaaue = 76766;
	bool vymambbexinmg = true;
	int vxsniztflekmkzb = 1281;
	double luyjryjxfgjv = 12127;
	int uakxwpnenahstxl = 2765;
	bool iriuavn = true;
	int gmaoed = 1001;
	if (true != true) {
		int mn;
		for (mn = 75; mn > 0; mn--) {
			continue;
		}
	}
	return 22001;
}

double pdkwpzr::uefltcaxvdbydplfbtrbi(double zoupeinmxqfuyfk, bool hfbazdfvlrb) {
	return 64031;
}

double pdkwpzr::murlcsxjpwbyqkwgvmckztp(bool hxqgq, int tqlfpdpxfpfvfs, int htdyjca, string oaoebaegybzqsdj, string ifxyhblfvfcvo, int fababoq) {
	int kxpsgfihuldsnbn = 3047;
	if (3047 != 3047) {
		int addsp;
		for (addsp = 16; addsp > 0; addsp--) {
			continue;
		}
	}
	if (3047 != 3047) {
		int bnixjudw;
		for (bnixjudw = 19; bnixjudw > 0; bnixjudw--) {
			continue;
		}
	}
	if (3047 != 3047) {
		int ougsstv;
		for (ougsstv = 47; ougsstv > 0; ougsstv--) {
			continue;
		}
	}
	if (3047 != 3047) {
		int hq;
		for (hq = 44; hq > 0; hq--) {
			continue;
		}
	}
	return 65668;
}

string pdkwpzr::mgbixxtvhvjresbwoyfrlytkr(bool nqvusiwuz, int bcbipsew) {
	bool ocqilqbm = false;
	string xhfuqbxva = "";
	bool dvrcxn = false;
	if (false == false) {
		int gbzmkus;
		for (gbzmkus = 28; gbzmkus > 0; gbzmkus--) {
			continue;
		}
	}
	if (false != false) {
		int dlreihgk;
		for (dlreihgk = 56; dlreihgk > 0; dlreihgk--) {
			continue;
		}
	}
	return string("aegiaqquvkykkwx");
}

void pdkwpzr::daumqsipbnubfkbvug(double oizxn, string mrtiolclxfi, int axvdccpnbrzem, double ufcqasucofaj) {
	string ceteirsrgeyqrk = "xgvbxsroasyvpkucmaljrotecllzlidwwklzmpmcabfccpuopvfegxwrwnldvdgjxfbhsytjmuenhgeubzebwgs";
	double nxtvfcv = 44473;
	string wxtnj = "lxl";
	double kvuwgxvbzkvvq = 7744;
	if (44473 == 44473) {
		int myie;
		for (myie = 25; myie > 0; myie--) {
			continue;
		}
	}
	if (44473 == 44473) {
		int yxbqcoaz;
		for (yxbqcoaz = 33; yxbqcoaz > 0; yxbqcoaz--) {
			continue;
		}
	}
	if (7744 == 7744) {
		int oxgdke;
		for (oxgdke = 50; oxgdke > 0; oxgdke--) {
			continue;
		}
	}
	if (string("xgvbxsroasyvpkucmaljrotecllzlidwwklzmpmcabfccpuopvfegxwrwnldvdgjxfbhsytjmuenhgeubzebwgs") == string("xgvbxsroasyvpkucmaljrotecllzlidwwklzmpmcabfccpuopvfegxwrwnldvdgjxfbhsytjmuenhgeubzebwgs")) {
		int nonye;
		for (nonye = 55; nonye > 0; nonye--) {
			continue;
		}
	}

}

double pdkwpzr::hknarldhltbshxenaz(bool xwhyiq, string tlknyn, double vcbocugxpz, bool vvdifjwum, double cdvxfcx, int jyspqnkmy, int vewdyixcvyj, int zjeav, bool raabsxanpbhablx) {
	double yrvqcwmaxxnyg = 65840;
	bool bbdtsq = true;
	string rtertedfbm = "fmihezbjmvuoxeixuzantcenlkodulrbxkwpnnkmuaqtdtjxzeiosbsedmdcrkvevziunzwxsapuoyrlyhmraaefpyshxdvz";
	double mxxquz = 10382;
	string bicdki = "ikfzeatoezrnbfavlyg";
	double glekqemgm = 15801;
	string vtljcj = "kblddxhlzlrzmpnhajjobkddoelhzhnwcqxzqescndxxbxpuhduubywoipgzznctvazwcqygskuzzpjojwhccleymxwoqihfunwn";
	string kzyownxngmyfn = "jcqqpexweuijxzlqgnqncjfxgjfshehdrtylfodbfnpvgyfwjcsdsiwkgdvomvmmrpegplylhfurkdguckntruhqrxxh";
	string udeofidksttg = "g";
	int iflqqsbvu = 1862;
	if (true == true) {
		int pi;
		for (pi = 100; pi > 0; pi--) {
			continue;
		}
	}
	return 55401;
}

bool pdkwpzr::ngtbwgyrqib(bool mffrtmldqrjzg, int zkppamjigyzdp, int uegdypa, bool oylcil, bool vhutq, int qeiitsy) {
	string cyotvasdisg = "vjbzvhoikzkekeayvcfxmvprygmijumyvxkwwrmhughlibvflrxbprofrau";
	string rfvxzn = "oukvruxwtdypawlcrfhsjj";
	bool vtcuceqpjc = false;
	int afghblri = 2383;
	string mzxzqkeckeh = "nxyjhamtxcozvkjnejyrygkzswmvrf";
	double hsygmpimpmc = 58214;
	bool xcgqtdpobui = true;
	if (true != true) {
		int vbdu;
		for (vbdu = 95; vbdu > 0; vbdu--) {
			continue;
		}
	}
	return false;
}

string pdkwpzr::atyzthumkrwvskesiysdukjm(double sieqdzdjhpzfmf, string kevsthkeskkvtx, int juwkndscdycrmf, bool ghnowexvuwv, double frnphxgdpnwfv, double mmwxlsyklufgoo, double knpamhsujm, bool inflmmcgdks) {
	bool dlibhhrms = true;
	bool dzhtcyltw = true;
	return string("ipwlehmiszeikiwg");
}

void pdkwpzr::kssfgygqdhdchxlexip(int lkumcspxrkjazdm, string hegtle, bool ltsta, int akjiur, string tattikqryz, double kqutkzikifexdjr, int bpgetc, bool xylnwe, double xbrecyucrsjngzm) {
	double lnqmhoibndiod = 8862;
	double hpfzppvjnuc = 40127;
	string irfpxgdbg = "vworjihpfjfcig";
	int ooztvyvwcjhclwj = 3409;
	int qdstaprzoyygjf = 7114;
	string zmucvmcyavfdjsw = "dehwdkxlrjwrzxjohvndbscmyturikhuku";
	string narah = "qbwkrglczteuvsojmcwcfzqzxzmefouffdtsluaiwuhzenoslhafjymbmvaomkhd";
	if (8862 == 8862) {
		int ka;
		for (ka = 1; ka > 0; ka--) {
			continue;
		}
	}
	if (string("vworjihpfjfcig") == string("vworjihpfjfcig")) {
		int cadjzdycvk;
		for (cadjzdycvk = 52; cadjzdycvk > 0; cadjzdycvk--) {
			continue;
		}
	}
	if (40127 == 40127) {
		int bzzsrxg;
		for (bzzsrxg = 44; bzzsrxg > 0; bzzsrxg--) {
			continue;
		}
	}
	if (string("qbwkrglczteuvsojmcwcfzqzxzmefouffdtsluaiwuhzenoslhafjymbmvaomkhd") != string("qbwkrglczteuvsojmcwcfzqzxzmefouffdtsluaiwuhzenoslhafjymbmvaomkhd")) {
		int xzpackaf;
		for (xzpackaf = 85; xzpackaf > 0; xzpackaf--) {
			continue;
		}
	}
	if (string("vworjihpfjfcig") == string("vworjihpfjfcig")) {
		int lkiti;
		for (lkiti = 82; lkiti > 0; lkiti--) {
			continue;
		}
	}

}

bool pdkwpzr::almnnrcyhlrgafxhtl(bool qvhkhddt) {
	double ccvzcpz = 4749;
	double neectjb = 2793;
	bool pclmprlar = true;
	int avycihq = 5353;
	double hobsbw = 14013;
	bool oehnjgk = false;
	int wsjmgjwk = 3480;
	int svpurlf = 3034;
	if (3034 != 3034) {
		int zf;
		for (zf = 59; zf > 0; zf--) {
			continue;
		}
	}
	if (true != true) {
		int izvjkrpbc;
		for (izvjkrpbc = 35; izvjkrpbc > 0; izvjkrpbc--) {
			continue;
		}
	}
	if (true == true) {
		int klwl;
		for (klwl = 71; klwl > 0; klwl--) {
			continue;
		}
	}
	return true;
}

pdkwpzr::pdkwpzr() {
	this->hknarldhltbshxenaz(false, string("dwpidhnsquxrlbxutzmejqnhxwnjoyglqrvgb"), 49844, false, 37480, 1377, 8561, 312, false);
	this->ngtbwgyrqib(true, 978, 1782, false, true, 464);
	this->atyzthumkrwvskesiysdukjm(54023, string("peltauqvtxmgyanrqgafgwbqnowenfofmked"), 3044, false, 61213, 18635, 59291, true);
	this->kssfgygqdhdchxlexip(4266, string("mieracwvpcyvwxkgpjnfidmdzgjmdhuginncmweditgberjxtnqhwfjztlythphxevfgjnwfscyryrzgamf"), false, 5158, string("zjhvephrryeqjzqddbndirrsfauwmqxcbzspcdbbmqvbxbozd"), 60144, 2958, true, 10408);
	this->almnnrcyhlrgafxhtl(false);
	this->uefltcaxvdbydplfbtrbi(17318, false);
	this->murlcsxjpwbyqkwgvmckztp(false, 2652, 2573, string("iangun"), string("briirtaizqbqlmgqgryhluybtqxyuibuxdbbgeufyszvasszztgqtmtkyiabgnhnojidc"), 163);
	this->mgbixxtvhvjresbwoyfrlytkr(true, 2402);
	this->daumqsipbnubfkbvug(33376, string("vkxfhjnhdbjkpavwqgoerpqzujrxuoeoblolklceyfcywsdvirbdtejgfzpncebnkaququsfohwnmxneiasvrfxq"), 1672, 22831);
	this->awapblvprhn();
	this->dmqiblusyzcbaaqubqhbaxqcq(7932, 16045, 617, string("uefsjvnjzvtaqxextitgqhqctrgqjp"), string("wed"), false, 35070, false);
	this->fsvhnorepfmrojtl(29774, 769);
	this->jkfeutigmkzroyxgtrtngo(6442, 1116);
	this->kxazrojgetbfrfgbcwifrrsb(47741, false, 9081, false, true, true, true);
	this->owxqmvsgfkcvxxrwfvrjuh(true, true, string("bqcanjfvqinaflxmxxzjbuiwmkugnaifsaktngcgswdztqxbsscfsjtqmqrtpattxceflelqdhvfa"), string("ssrrsuowsgzaluktsxjxvjjwgiq"), false, string("rdfyglyccbfiitrubpdkcuqqu"), string("gdwbslyf"), 3233, 252);
}

















