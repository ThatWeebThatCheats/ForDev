#include "antiaim.hpp"
#include "../../options.hpp"
#include "../../Helpers/math.hpp"
#include "../../hooks.hpp"
#include "../../Helpers/draw_manager.hpp"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif


#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

void CAntiAim::CreateMove(CUserCmd* cmd, bool& bSendPacket)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	int movetype = g_LocalPlayer->m_nMoveType();

	if (movetype == MOVETYPE_FLY || movetype == MOVETYPE_NOCLIP || cmd->buttons & IN_USE || cmd->buttons & IN_GRENADE1 || cmd->buttons & IN_GRENADE2)
		return;

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	if (weapon->m_flNextPrimaryAttack() - g_GlobalVars->curtime < g_GlobalVars->interval_per_tick && (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2))
		return;

	if (movetype == MOVETYPE_LADDER)
	{
		static bool last = false;
		bSendPacket = last;
		last = !last;
		return;
	}

	if (weapon->IsGrenade() && weapon->m_fThrowTime() > 0.1f)
	{
		bSendPacket = false;
		return;
	}

	DoAntiAim(cmd, bSendPacket);
}

float randnum2(int Min, int Max)
{
	return ((rand() % (Max - Min)) + Min);
}

float get_feet_yaw()
{
	auto local_player = static_cast<C_BasePlayer*>(Interfaces::EntityList->GetClientEntity(Interfaces::Engine->GetLocalPlayer()));

	if (!local_player)
		return 0.f;

	auto state = local_player->GetBasePlayerAnimState();

	float current_feet_yaw = state->m_flGoalFeetYaw;

	if (current_feet_yaw >= -360)
		current_feet_yaw = min(current_feet_yaw, 360.f);

	return current_feet_yaw;
}

float setup_desync(int mode, bool& bSendPacket, CUserCmd* cmd) 
{
	float max_delta = 0;

	if (!bSendPacket)
		return 0;

	auto get_max_desync_delta_reversed = [&]() -> float 
	{
		auto animstate = G::LocalPlayer->GetBasePlayerAnimState();
		float v49;
		float v46;
		float v51;

		if (animstate->m_flFeetSpeedForwardsOrSideWays >= 0.0)
			v46 = fminf(animstate->m_flFeetSpeedForwardsOrSideWays, 1.0);
		else
			v46 = 0.0;

		float v47 = (float)((animstate->m_flStopToFullRunningFraction * -0.30000001f) - 0.19999999f) * v46;
		v49 = v47 + 1.0;
		if (animstate->m_fDuckAmount > 0.0)
		{
			if (animstate->m_flFeetSpeedForwardsOrSideWays >= 0.0)
				v51 = fminf(animstate->m_flFeetSpeedForwardsOrSideWays, 1.0);
			else
				v51 = 0.0;
			float v52 = animstate->m_fDuckAmount * v51;
			v49 = v49 + (float)(v52 * (float)(0.5 - v49));
		}

		float v53 = *(float*)(animstate + 0x334) * v49;
		return v53;
	};

	switch (mode)
	{
	case 1:  // balance desync
	{
		if (cmd->command_number % 3)
		{
			if (get_max_desync_delta_reversed() >= -58 + randnum2(-30, 30) || get_max_desync_delta_reversed() <= 58 + randnum2(-30, 30))
				max_delta = get_max_desync_delta_reversed();
			else
				max_delta = 58 + randnum2(-30, 30);
		}

		break;
	}

	case 2:  // stretch desync
	{
		if (cmd->command_number % 3)
			max_delta = 58 + randnum2(-30, 30);

		break;
	}

	case 3:  // fake jitter desync
	{
		if (cmd->command_number % 3)
		{
			max_delta = rand() % -58 + 58 + randnum2(-30, 30);
		}

		break;
	}

	case 4:  // switch desync
	{
		if (cmd->command_number % 3)
		{
			max_delta = (-58 / 2.f + std::fmodf(g_GlobalVars->curtime * 150, 58) + randnum2(-30, 30));
		}

		break;
	}

	case 5: // testing desync
	{
		if (cmd->command_number % 3)
		{
			max_delta = get_feet_yaw() + randnum2(-30, 30);
		}

		break;
	}
	}

	*G::LocalPlayer->GetBasePlayerAnimState()->feetyaw() = -118;
	return max_delta;
}

void CAntiAim::Yaw(CUserCmd* cmd, bool& bSendPacket)
{
	bool Moving = g_LocalPlayer->m_vecVelocity().Length2D() > 0.1;
	bool InAir = !(g_LocalPlayer->m_fFlags() & FL_ONGROUND);
	bool Standing = !Moving && !InAir && Globals::LocalPlayer->m_fFlags() & FL_ONGROUND && Globals::LocalPlayer->m_vecVelocity().Length2D() < 0.1f;

	if (Standing && !Moving && !InAir)
	{
		if (bSendPacket && bSendPacket == true)
		{
			if (Vars.ragebot_antiaim_desync)
			{
				if (cmd->command_number % 3)
				{
					cmd->viewangles.yaw = cmd->viewangles.yaw + setup_desync(Vars.ragebot_antiaim_desynctype, bSendPacket, cmd);
				}
			}
		}
		else if (!bSendPacket && bSendPacket == false)
		{
			switch (Vars.ragebot_antiaim_yaw)
			{
			case 0:
				break;

			case 1:
				Freestanding(g_LocalPlayer, cmd);
				break;

			case 2:
				cmd->viewangles.yaw += 180.f;
				break;
			}
		}
	}
	else if (Moving && !InAir && !Standing)
	{
		if (bSendPacket && bSendPacket == true)
		{
			if (Vars.ragebot_antiaim_desync)
			{
				if (cmd->command_number % 3)
				{
					cmd->viewangles.yaw = cmd->viewangles.yaw + setup_desync(Vars.ragebot_antiaim_desynctype, bSendPacket, cmd);
				}
			}
		}
		else if (!bSendPacket && bSendPacket == false)
		{
			switch (Vars.ragebot_antiaim_yaw_move)
			{
			case 0:
				break;

			case 1:
				Freestanding(g_LocalPlayer, cmd);
				break;

			case 2:
				cmd->viewangles.yaw += 180.f;
				break;
			}
		}
	}
	else
	{
		switch (Vars.ragebot_antiaim_yaw_air)
		{
		case 0:
			break;

		case 1:
			cmd->viewangles.yaw += 180.f;
			break;
		}
	}
}

void CAntiAim::Pitch(CUserCmd* cmd)
{
	bool Moving = g_LocalPlayer->m_vecVelocity().Length2D() > 0.1;
	bool InAir = !(g_LocalPlayer->m_fFlags() & FL_ONGROUND);
	bool Standing = !Moving && !InAir && Globals::LocalPlayer->m_fFlags() & FL_ONGROUND && Globals::LocalPlayer->m_vecVelocity().Length2D() < 0.1f;

	if (Standing && !Moving && !InAir)
	{
		switch (Vars.ragebot_antiaim_pitch)
		{
		case 0:
			break;

		case 1:
			cmd->viewangles.pitch = 82.f;
			break;

		case 2:
			cmd->viewangles.pitch = 90.f;
			break;

		case 3:
			cmd->viewangles.pitch = -90.f;
			break;
		}
	}
	else if (Moving && !Standing && !InAir)
	{
		switch (Vars.ragebot_antiaim_pitch_move)
		{
		case 0:
			break;

		case 1:
			cmd->viewangles.pitch = 82.f;
			break;

		case 2:
			cmd->viewangles.pitch = 90.f;
			break;

		case 3:
			cmd->viewangles.pitch = -90.f;
			break;
		}
	}
	else
	{
		switch (Vars.ragebot_antiaim_pitch_air)
		{
		case 0:
			break;

		case 1:
			cmd->viewangles.pitch = 82.f;
			break;

		case 2:
			cmd->viewangles.pitch = 90.f;
			break;

		case 3:
			cmd->viewangles.pitch = -90.f;
			break;
		}
	}
}

void CAntiAim::DoAntiAim(CUserCmd* cmd, bool& bSendPacket)
{
	if (!Utils::IsInGame())
		return;

	if (!Globals::LocalPlayer)
		return;

	if (!Globals::LocalPlayer->IsAlive())
		return;

	Pitch(cmd);
	Yaw(cmd, bSendPacket);
}

void CAntiAim::Desync(CUserCmd* cmd, bool& bSendPacket)
{

}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class cyxhpmp {
public:
	bool kxlowxhujo;
	bool asvehyysgvfxkre;
	int mxxsicdv;
	int xgnfzmdgabsm;
	cyxhpmp();
	bool myglcbvljn(int axpfbkd);
	void gswvyjqtssqewxkhwkpzdngxl(bool clptmrgcvzlovh, double lvtavlyzzdh);
	int tszixwsrjfrgk(string xsjejj, string xnemwwoyclqlubt, bool xwlykmxh, string drjusgkhyknrht, string tskyvfyhh);
	int qohyqzgokguedzyyujxsjxsgn(bool jdnhzzdvalar, bool lbqtohirdddsvs, int ifrduxipxq, bool hnashunvxjcfpgy, bool ehvfht, bool afbnzplzc, int vgmexste, bool mrdvucls, bool anxvp);
	string ciapynczxagkyqqqgcej(double zvefsmnak, bool vrlfmlubmk, double kdxmuiohiakqu, bool hftaxkfiaueab, string qduurdld);
	bool ayxaptomnefvcwkdlwfczves(string earzs, string srpncww, bool zaunshcixzandy, double bffzw, int ujzbggvqjcf);
	string pthmbsnzwucryruubzwqzrtm();
	string teswunuubp(int iqedu, double loqxtynrf);
	int gpobuxpbksgkmtaumyq(int vyrgekn, bool yhyyrly, bool wdypnleo, string viqmabwd);
	string dwdwxyyxdtokdhornlolqqgse(int hoyxmhtlwfhu, bool qtoajbkymuju);

protected:
	double qroqengp;

	int jrbbsexsruf(bool smlgbezgiavu, double ihbgykbymtu, int tdqrieheswukcz, bool zdmqgbc, int sowgmaru, int xmoyfuhu, double csvccl, string wimsu, int zmadej);
	string nmfknacynnlxbvxl(string tiyiutufxc, double stdiprxkizojjm, double bjlknsarmwhmlg, double uudnyyujuoyrgz, int jizccuyozabrfbi, int ltsds, int tbqjidn, double fxrnntntupmk, int dzyzklvxqeped, int fecrmfw);
	bool rygfnurcpdve(int yokrsuae, int rkhugetvuwtr);
	string wdxoxlpduynigniumqxmjbcpj(double nmkrzlwulhu, bool qfwvom, string xfddmfz, bool okcxdeccdv, double qlurhyou, bool vvfakrebiaorwuz, bool wkevnjh, int zmvbjwgtch);
	void lwozdwgxyi();
	bool esukqbtznzwt(double oeusesgkf, string ueukguoduixbo, string bkninkqp, bool imcesptdzny, bool dwlkgwoqdqy, int ucvahcyzkjhhi);
	double raalidxpfnxktkl(string xtlnxarwnsv, string kfojzwzhlf, bool pzgnkythrgzkht, string sypxepwirsvl);
	bool leywlfhyalsuieevmy();
	bool kaxhucmddradcxt(bool slfhlwhicm, double xutvxrvbfurvkhp, int pivir, string etxvs, bool masvjfswu);
	void ibdwwovbxlwwdwsek(string gaksji, bool wrbspahww, string irjehevxipbm, double gvlcmzfapnzle, double mjbhujsp);

private:
	double naoqeqit;
	bool htsdou;

	bool wufrgizxleoohhyqn(double niamjruvynchv, double ltuxqfkbznompae, double alsphjlkxw, double zggzczvt, int iawchekpkg);
	bool lyzmudzger(int qngjzxy, bool rnhrhmclkojkx, double kygvnwvmv, bool xgbaoaegccfu, bool cpefyuwepttfg);
	bool neqdwitmwinvzxnq(int snfwjkecgpxpfqg, int zmxstmee, double owbtl, double dbyxntpjcobe, string jnyvo, double vchrbwphmuabdj);
	double qmyrfqnhgapnvukksvcttcqb(int fuagce, int nquvflz, int nemtempp, int vsdrqahak, double svrxlviubqvdd, bool tbtjzcbhdmmevc, bool fykyuxsgvngid, bool gaoielxxus, double epjncnhxihnvi, int rifqoihvpe);
	string htbhiukvsdtogxy(string xinyrfezofz, double hbldbgfwbehptj, double tgiawfuqljaqby, int mwfmmrtatnjytsa, int zgcliojscoe, double jjxvgwgwronycd, int bipwhkopdu, string kwbvgxtbcazdr, string ysyasled, bool mgppjuqikfdd);
	bool aynvwlkzsjwlpblnamkro(bool izwbilvcpk, int fzolegcsaj, bool rqzfxgh, bool dwaphgnlsulrf, string ljjlnbkxd, string yuqpmfz, bool arptdtjik, double uprljatjl);
	double xipnjqkwdahikqmtwuns();
	bool nbezbxmuxycwuvd(int vcedt, bool essrocspajketw, string oqoqsmkzqp, double mzxhqixm, int qxyayyxl, int jxqcgqiak, int gqkzryyqrlxqtth, double kzghdxvmfsq, bool zodqyma, int khdcervfer);

};



bool cyxhpmp::wufrgizxleoohhyqn(double niamjruvynchv, double ltuxqfkbznompae, double alsphjlkxw, double zggzczvt, int iawchekpkg) {
	string evjhmgaowbmj = "owyklmghhokbefimjkxppoymhxbmrhzhryyhtbwnraiexayepkzucflfnwfpacqmlcotnaub";
	bool zxgxgs = false;
	if (false != false) {
		int bb;
		for (bb = 1; bb > 0; bb--) {
			continue;
		}
	}
	if (false != false) {
		int fuy;
		for (fuy = 14; fuy > 0; fuy--) {
			continue;
		}
	}
	if (string("owyklmghhokbefimjkxppoymhxbmrhzhryyhtbwnraiexayepkzucflfnwfpacqmlcotnaub") == string("owyklmghhokbefimjkxppoymhxbmrhzhryyhtbwnraiexayepkzucflfnwfpacqmlcotnaub")) {
		int gbgw;
		for (gbgw = 22; gbgw > 0; gbgw--) {
			continue;
		}
	}
	if (false != false) {
		int sanfis;
		for (sanfis = 95; sanfis > 0; sanfis--) {
			continue;
		}
	}
	if (false != false) {
		int yve;
		for (yve = 78; yve > 0; yve--) {
			continue;
		}
	}
	return true;
}

bool cyxhpmp::lyzmudzger(int qngjzxy, bool rnhrhmclkojkx, double kygvnwvmv, bool xgbaoaegccfu, bool cpefyuwepttfg) {
	return true;
}

bool cyxhpmp::neqdwitmwinvzxnq(int snfwjkecgpxpfqg, int zmxstmee, double owbtl, double dbyxntpjcobe, string jnyvo, double vchrbwphmuabdj) {
	double dexqhi = 5626;
	string gzmlbwrxaadt = "ayurjxnzirmueuovstqvrjxsbuyplrgdlgiaojjccgglqfbpaaiqlkulbtvcoieeikqsnky";
	bool pusqiakzpn = true;
	int iwpjdwcyqz = 975;
	bool msdikvdiplpuu = false;
	double mlnnxmzaypojv = 18732;
	double addcgpecczfnp = 6741;
	bool urhqksqzsvrtbx = false;
	if (string("ayurjxnzirmueuovstqvrjxsbuyplrgdlgiaojjccgglqfbpaaiqlkulbtvcoieeikqsnky") == string("ayurjxnzirmueuovstqvrjxsbuyplrgdlgiaojjccgglqfbpaaiqlkulbtvcoieeikqsnky")) {
		int thbmz;
		for (thbmz = 60; thbmz > 0; thbmz--) {
			continue;
		}
	}
	return true;
}

double cyxhpmp::qmyrfqnhgapnvukksvcttcqb(int fuagce, int nquvflz, int nemtempp, int vsdrqahak, double svrxlviubqvdd, bool tbtjzcbhdmmevc, bool fykyuxsgvngid, bool gaoielxxus, double epjncnhxihnvi, int rifqoihvpe) {
	string fwiuhbualkfteqv = "xpcibeszeczmbrsvgwriexhgsksyjfgqpeqolfjwzrewbboblxtwozofknjtjflqjlsdcxwxkischowudnuxrbovchzpdvlhncx";
	string pxmlued = "hputmngeycuqwbnserfgynsbwpsxaampghkp";
	double yzfuyqjmcxhypuq = 17794;
	bool mkpivrxgbdxbk = false;
	double ckpmhekjnt = 1428;
	string gadkasvlmiiru = "foyockpgctnpbojvvqndunvnhlekgrmecgbigfooqktadgqhwdttbycqqfijgxiz";
	int qgkapaulgwvyqzz = 1597;
	double thyqlsvmoezanke = 33677;
	if (33677 != 33677) {
		int wsq;
		for (wsq = 95; wsq > 0; wsq--) {
			continue;
		}
	}
	if (string("xpcibeszeczmbrsvgwriexhgsksyjfgqpeqolfjwzrewbboblxtwozofknjtjflqjlsdcxwxkischowudnuxrbovchzpdvlhncx") == string("xpcibeszeczmbrsvgwriexhgsksyjfgqpeqolfjwzrewbboblxtwozofknjtjflqjlsdcxwxkischowudnuxrbovchzpdvlhncx")) {
		int mkz;
		for (mkz = 98; mkz > 0; mkz--) {
			continue;
		}
	}
	if (33677 != 33677) {
		int igsnleiwfh;
		for (igsnleiwfh = 8; igsnleiwfh > 0; igsnleiwfh--) {
			continue;
		}
	}
	if (false == false) {
		int rwwo;
		for (rwwo = 44; rwwo > 0; rwwo--) {
			continue;
		}
	}
	return 75694;
}

string cyxhpmp::htbhiukvsdtogxy(string xinyrfezofz, double hbldbgfwbehptj, double tgiawfuqljaqby, int mwfmmrtatnjytsa, int zgcliojscoe, double jjxvgwgwronycd, int bipwhkopdu, string kwbvgxtbcazdr, string ysyasled, bool mgppjuqikfdd) {
	string xmxpbsuyhatgmbu = "yrbbcloayuxjukyagptq";
	double ueyqpke = 51821;
	double hgumawgv = 57166;
	if (57166 != 57166) {
		int zslrumszq;
		for (zslrumszq = 8; zslrumszq > 0; zslrumszq--) {
			continue;
		}
	}
	if (51821 == 51821) {
		int fr;
		for (fr = 36; fr > 0; fr--) {
			continue;
		}
	}
	if (57166 != 57166) {
		int hi;
		for (hi = 29; hi > 0; hi--) {
			continue;
		}
	}
	if (57166 != 57166) {
		int gvmcncwlhi;
		for (gvmcncwlhi = 15; gvmcncwlhi > 0; gvmcncwlhi--) {
			continue;
		}
	}
	if (57166 != 57166) {
		int kbmad;
		for (kbmad = 23; kbmad > 0; kbmad--) {
			continue;
		}
	}
	return string("dgwzosyutq");
}

bool cyxhpmp::aynvwlkzsjwlpblnamkro(bool izwbilvcpk, int fzolegcsaj, bool rqzfxgh, bool dwaphgnlsulrf, string ljjlnbkxd, string yuqpmfz, bool arptdtjik, double uprljatjl) {
	int sjdtqglo = 158;
	int kpizooorsnc = 1;
	bool vdrzwvqtnuee = false;
	string ddolxpxhg = "tzyfqcyinzczhhghsvvanbiwwbocwghhlpobgitkwvjvheko";
	bool ypnnqzhlvij = false;
	bool eycor = false;
	if (string("tzyfqcyinzczhhghsvvanbiwwbocwghhlpobgitkwvjvheko") == string("tzyfqcyinzczhhghsvvanbiwwbocwghhlpobgitkwvjvheko")) {
		int cyqec;
		for (cyqec = 59; cyqec > 0; cyqec--) {
			continue;
		}
	}
	if (string("tzyfqcyinzczhhghsvvanbiwwbocwghhlpobgitkwvjvheko") != string("tzyfqcyinzczhhghsvvanbiwwbocwghhlpobgitkwvjvheko")) {
		int qtymtgqv;
		for (qtymtgqv = 83; qtymtgqv > 0; qtymtgqv--) {
			continue;
		}
	}
	if (string("tzyfqcyinzczhhghsvvanbiwwbocwghhlpobgitkwvjvheko") != string("tzyfqcyinzczhhghsvvanbiwwbocwghhlpobgitkwvjvheko")) {
		int kjpd;
		for (kjpd = 24; kjpd > 0; kjpd--) {
			continue;
		}
	}
	if (string("tzyfqcyinzczhhghsvvanbiwwbocwghhlpobgitkwvjvheko") != string("tzyfqcyinzczhhghsvvanbiwwbocwghhlpobgitkwvjvheko")) {
		int aiwtfg;
		for (aiwtfg = 74; aiwtfg > 0; aiwtfg--) {
			continue;
		}
	}
	if (158 == 158) {
		int expvawc;
		for (expvawc = 40; expvawc > 0; expvawc--) {
			continue;
		}
	}
	return true;
}

double cyxhpmp::xipnjqkwdahikqmtwuns() {
	double bzwnojwiudhbdbc = 6409;
	double qrhhfvl = 73588;
	int amzcy = 448;
	string bghdndlrcv = "ilmjpxcutmyfdmqkenmmvsklwyeaebelioxcwjujotycevixwnhgnch";
	int prbskxmgs = 4127;
	int nivygslbejgeg = 1577;
	double jsser = 87032;
	double jzgvdvrhaemgqjl = 6927;
	string zhncoaaigagsvj = "mokhcyeqxkzfcfwqzkciwoaappwpwqmxemlklnlrwfpdcqywdxhedfdygocwgsovnlgyyibf";
	if (73588 == 73588) {
		int eebimbjoo;
		for (eebimbjoo = 98; eebimbjoo > 0; eebimbjoo--) {
			continue;
		}
	}
	return 37264;
}

bool cyxhpmp::nbezbxmuxycwuvd(int vcedt, bool essrocspajketw, string oqoqsmkzqp, double mzxhqixm, int qxyayyxl, int jxqcgqiak, int gqkzryyqrlxqtth, double kzghdxvmfsq, bool zodqyma, int khdcervfer) {
	int xbmebcixlhixvwf = 4022;
	string ykgkrehgkabr = "omjdeazodaweldmyeuuqthmvqtuhudgbftzhremncwyflvpujr";
	double pbiguuftpant = 32082;
	bool xihbbqsrn = false;
	double wqijbf = 66677;
	bool rajjupmxoz = true;
	int rsimbb = 6223;
	string fyqalpeurc = "vsftdkntvgkvdtxojoyqbgbdruvldobmpgsycrjtlacj";
	int zrlcwo = 561;
	if (6223 == 6223) {
		int jkoucy;
		for (jkoucy = 67; jkoucy > 0; jkoucy--) {
			continue;
		}
	}
	if (66677 == 66677) {
		int mlbsyyivu;
		for (mlbsyyivu = 4; mlbsyyivu > 0; mlbsyyivu--) {
			continue;
		}
	}
	if (false != false) {
		int ct;
		for (ct = 80; ct > 0; ct--) {
			continue;
		}
	}
	if (561 != 561) {
		int hxpymc;
		for (hxpymc = 88; hxpymc > 0; hxpymc--) {
			continue;
		}
	}
	return true;
}

int cyxhpmp::jrbbsexsruf(bool smlgbezgiavu, double ihbgykbymtu, int tdqrieheswukcz, bool zdmqgbc, int sowgmaru, int xmoyfuhu, double csvccl, string wimsu, int zmadej) {
	double wkyeglmjxethmlb = 12792;
	bool cnydhretsdop = false;
	string rwswfavztiomzs = "uzsesrybwvmhle";
	double qxdxgmjy = 3052;
	string emisriouqu = "ygsrsbzoceyokepuovnjzrnbdjbjidqozdhhhpl";
	bool wosylsgodcw = false;
	double udzbplmb = 69351;
	if (string("uzsesrybwvmhle") != string("uzsesrybwvmhle")) {
		int pyxsmapyf;
		for (pyxsmapyf = 60; pyxsmapyf > 0; pyxsmapyf--) {
			continue;
		}
	}
	if (12792 != 12792) {
		int gsuwutpr;
		for (gsuwutpr = 50; gsuwutpr > 0; gsuwutpr--) {
			continue;
		}
	}
	if (3052 == 3052) {
		int ddxgjmwemz;
		for (ddxgjmwemz = 75; ddxgjmwemz > 0; ddxgjmwemz--) {
			continue;
		}
	}
	if (12792 == 12792) {
		int mvs;
		for (mvs = 56; mvs > 0; mvs--) {
			continue;
		}
	}
	return 17122;
}

string cyxhpmp::nmfknacynnlxbvxl(string tiyiutufxc, double stdiprxkizojjm, double bjlknsarmwhmlg, double uudnyyujuoyrgz, int jizccuyozabrfbi, int ltsds, int tbqjidn, double fxrnntntupmk, int dzyzklvxqeped, int fecrmfw) {
	int bntbpwjr = 4429;
	double ugqpui = 43679;
	double duxfn = 15834;
	double lvlifyo = 37646;
	double zmqowmlcio = 1275;
	double cayjvddwc = 12762;
	string jyafggdob = "oidsjsayncelpcinibdmfheybbwthtwegigishatnbzehdsasifbw";
	int kwqoqjmklapdmvi = 100;
	bool qolzzh = true;
	if (1275 == 1275) {
		int rmevq;
		for (rmevq = 5; rmevq > 0; rmevq--) {
			continue;
		}
	}
	if (4429 == 4429) {
		int jhenbb;
		for (jhenbb = 37; jhenbb > 0; jhenbb--) {
			continue;
		}
	}
	if (12762 != 12762) {
		int rhsity;
		for (rhsity = 96; rhsity > 0; rhsity--) {
			continue;
		}
	}
	if (1275 != 1275) {
		int pm;
		for (pm = 30; pm > 0; pm--) {
			continue;
		}
	}
	return string("jhvvbsppmjwomcu");
}

bool cyxhpmp::rygfnurcpdve(int yokrsuae, int rkhugetvuwtr) {
	bool yuiowjamjb = true;
	double fkfennnt = 5909;
	string wghezbsyydomih = "yrzussnbpnwajbyskqpvycxkrvgtstcjrntlwgyspozmqojfqmrpklxeusezytcqpfccevzyywdx";
	if (5909 != 5909) {
		int nlfq;
		for (nlfq = 36; nlfq > 0; nlfq--) {
			continue;
		}
	}
	if (5909 == 5909) {
		int tmikmi;
		for (tmikmi = 46; tmikmi > 0; tmikmi--) {
			continue;
		}
	}
	if (true != true) {
		int trvmpfr;
		for (trvmpfr = 80; trvmpfr > 0; trvmpfr--) {
			continue;
		}
	}
	if (true != true) {
		int cw;
		for (cw = 87; cw > 0; cw--) {
			continue;
		}
	}
	if (true == true) {
		int tgmhkt;
		for (tgmhkt = 39; tgmhkt > 0; tgmhkt--) {
			continue;
		}
	}
	return false;
}

string cyxhpmp::wdxoxlpduynigniumqxmjbcpj(double nmkrzlwulhu, bool qfwvom, string xfddmfz, bool okcxdeccdv, double qlurhyou, bool vvfakrebiaorwuz, bool wkevnjh, int zmvbjwgtch) {
	bool tphjoghi = true;
	double ixvawz = 58294;
	double jvxmnhgqysd = 7049;
	bool vqpvlnftmwerpnu = true;
	string wixaqcc = "gvwghlivalfjazxjoledpbomhlgxzjqyudpcmmwbhxdmnnzccgcbkohfgzejlerfxmqtqrxtsdiwuqstyhnazdusaslfbiyo";
	double vpckgwyiaz = 7606;
	int ldxmxbtfopoc = 831;
	if (true == true) {
		int gowlvamp;
		for (gowlvamp = 78; gowlvamp > 0; gowlvamp--) {
			continue;
		}
	}
	if (true == true) {
		int fuidljjzb;
		for (fuidljjzb = 69; fuidljjzb > 0; fuidljjzb--) {
			continue;
		}
	}
	return string("nkikzfjmviiurd");
}

void cyxhpmp::lwozdwgxyi() {
	string wcdmsomzgbau = "hnefekoyfrvzlerzifxvmjzbksjvpogvphrmcyaafqfxexinlyeimbu";
	bool ywdwwxam = true;
	bool hddxpbjn = true;
	int pvqjmyzieqkipq = 3812;
	bool vznwzqerwxhyieo = false;
	string cldsaupahayfeta = "jwkwhjtjsvfsmovtjlcopfbfprnwvbslbnmlyhmd";
	string pjxopbrb = "suvipoweenblzndmynesdgotaewkdgadatrk";
	if (string("jwkwhjtjsvfsmovtjlcopfbfprnwvbslbnmlyhmd") != string("jwkwhjtjsvfsmovtjlcopfbfprnwvbslbnmlyhmd")) {
		int hql;
		for (hql = 84; hql > 0; hql--) {
			continue;
		}
	}
	if (false == false) {
		int afjjgu;
		for (afjjgu = 68; afjjgu > 0; afjjgu--) {
			continue;
		}
	}
	if (string("jwkwhjtjsvfsmovtjlcopfbfprnwvbslbnmlyhmd") == string("jwkwhjtjsvfsmovtjlcopfbfprnwvbslbnmlyhmd")) {
		int zq;
		for (zq = 48; zq > 0; zq--) {
			continue;
		}
	}
	if (true == true) {
		int hodwvy;
		for (hodwvy = 15; hodwvy > 0; hodwvy--) {
			continue;
		}
	}
	if (false == false) {
		int dia;
		for (dia = 25; dia > 0; dia--) {
			continue;
		}
	}

}

bool cyxhpmp::esukqbtznzwt(double oeusesgkf, string ueukguoduixbo, string bkninkqp, bool imcesptdzny, bool dwlkgwoqdqy, int ucvahcyzkjhhi) {
	double rroydozmw = 28745;
	bool hhczoxkwsfwk = true;
	string ougsjoepzypg = "vfljoadjvgueaafzptjrmmvdfwgdtaitcmetcjleopdsccwcpbpogqbhqpaechbuvlwaxkeixwaqvkm";
	double puhekrigdwse = 8993;
	string wsdxctxazn = "wewcwvkhfppglzschvmoqhjbdkhnmjhexlkifdpbbdzmdglbflebhqxrmhubavghdbpzhf";
	bool phjsns = false;
	string leednsbofh = "zadkzcuofnjeabczzivzioihxpvtwgsmefskdxzfngzylsrjhgnwxgqompjelgencgedjoyshuzhsgghty";
	double xrnmyfokg = 23371;
	string gdjxei = "cbxwerivkenluawlddlpchxbxcvkhwhqllpfgtlouvwlhjnjgeshatsquihyungfkyx";
	if (string("vfljoadjvgueaafzptjrmmvdfwgdtaitcmetcjleopdsccwcpbpogqbhqpaechbuvlwaxkeixwaqvkm") == string("vfljoadjvgueaafzptjrmmvdfwgdtaitcmetcjleopdsccwcpbpogqbhqpaechbuvlwaxkeixwaqvkm")) {
		int xodr;
		for (xodr = 24; xodr > 0; xodr--) {
			continue;
		}
	}
	return true;
}

double cyxhpmp::raalidxpfnxktkl(string xtlnxarwnsv, string kfojzwzhlf, bool pzgnkythrgzkht, string sypxepwirsvl) {
	string abjaqzxk = "imgrlerqlvbasgxxbhvxetvnxmphxequjlsmhjujwuycjjybxmhlxxgqdgupypqirzmvhnpgpzjmsixyimfonmuakwo";
	string rscbx = "mgvjtvqfjrrtnsjpopnaluesuyzlfikhgcznfnzgopqosnmstskbsnsdzldsxpqvmfxdhlhsplbszoqigqzfvxyh";
	double ffspyzqkwrerjdr = 9619;
	bool cuwcubhknlj = true;
	int rbdgjaoxnznptsm = 799;
	if (9619 != 9619) {
		int vmnhmowmn;
		for (vmnhmowmn = 43; vmnhmowmn > 0; vmnhmowmn--) {
			continue;
		}
	}
	if (string("imgrlerqlvbasgxxbhvxetvnxmphxequjlsmhjujwuycjjybxmhlxxgqdgupypqirzmvhnpgpzjmsixyimfonmuakwo") != string("imgrlerqlvbasgxxbhvxetvnxmphxequjlsmhjujwuycjjybxmhlxxgqdgupypqirzmvhnpgpzjmsixyimfonmuakwo")) {
		int stiwqsu;
		for (stiwqsu = 28; stiwqsu > 0; stiwqsu--) {
			continue;
		}
	}
	return 29219;
}

bool cyxhpmp::leywlfhyalsuieevmy() {
	int prsmxrlqkpf = 1717;
	bool kvzswfos = false;
	bool ygsxn = false;
	bool trknmmuiadc = false;
	if (1717 != 1717) {
		int ern;
		for (ern = 21; ern > 0; ern--) {
			continue;
		}
	}
	if (false != false) {
		int lvuglo;
		for (lvuglo = 0; lvuglo > 0; lvuglo--) {
			continue;
		}
	}
	if (false == false) {
		int oicva;
		for (oicva = 43; oicva > 0; oicva--) {
			continue;
		}
	}
	return true;
}

bool cyxhpmp::kaxhucmddradcxt(bool slfhlwhicm, double xutvxrvbfurvkhp, int pivir, string etxvs, bool masvjfswu) {
	string vlhgaoeted = "fugicvgzqwsanlnbxohmydprwhwtjvyudzoexwrnplzgkmdioylujnnjzyq";
	double jmbukcsszyfvmki = 37508;
	int gwpuvureagkyef = 1978;
	double yepbjfvzmwbp = 35471;
	double tpkvzxzljlamo = 35375;
	string arveqand = "rnk";
	int pbqdyac = 2047;
	bool qdxfslxny = false;
	int wuobw = 1484;
	if (2047 != 2047) {
		int pcohj;
		for (pcohj = 31; pcohj > 0; pcohj--) {
			continue;
		}
	}
	if (2047 != 2047) {
		int qekmolph;
		for (qekmolph = 9; qekmolph > 0; qekmolph--) {
			continue;
		}
	}
	if (37508 != 37508) {
		int nepjbmoz;
		for (nepjbmoz = 22; nepjbmoz > 0; nepjbmoz--) {
			continue;
		}
	}
	if (37508 == 37508) {
		int mmmv;
		for (mmmv = 90; mmmv > 0; mmmv--) {
			continue;
		}
	}
	if (2047 != 2047) {
		int goa;
		for (goa = 87; goa > 0; goa--) {
			continue;
		}
	}
	return true;
}

void cyxhpmp::ibdwwovbxlwwdwsek(string gaksji, bool wrbspahww, string irjehevxipbm, double gvlcmzfapnzle, double mjbhujsp) {
	int uhbmbr = 3113;
	double kaqhdaieklprjf = 17980;
	bool oyohujcagtwehh = false;
	int hnqjlywmj = 2585;
	bool ryznnqex = true;
	string yuhpumcwubnwkg = "atnyqhmdwdguyjrrcbhyfpdvokxttnuevnnpyosguwhs";
	bool deoymahirnrhjru = true;
	string kxbldyjlpyf = "znvqzhrfoxknlnghdpyoxpwpdoamjfvdwojdaafgmiuvusuxqumbntz";
	if (string("znvqzhrfoxknlnghdpyoxpwpdoamjfvdwojdaafgmiuvusuxqumbntz") != string("znvqzhrfoxknlnghdpyoxpwpdoamjfvdwojdaafgmiuvusuxqumbntz")) {
		int vkhuaztb;
		for (vkhuaztb = 13; vkhuaztb > 0; vkhuaztb--) {
			continue;
		}
	}
	if (string("atnyqhmdwdguyjrrcbhyfpdvokxttnuevnnpyosguwhs") == string("atnyqhmdwdguyjrrcbhyfpdvokxttnuevnnpyosguwhs")) {
		int vbgfhhqzoq;
		for (vbgfhhqzoq = 30; vbgfhhqzoq > 0; vbgfhhqzoq--) {
			continue;
		}
	}
	if (true != true) {
		int ogekvtv;
		for (ogekvtv = 64; ogekvtv > 0; ogekvtv--) {
			continue;
		}
	}
	if (string("atnyqhmdwdguyjrrcbhyfpdvokxttnuevnnpyosguwhs") != string("atnyqhmdwdguyjrrcbhyfpdvokxttnuevnnpyosguwhs")) {
		int txnhgljhjl;
		for (txnhgljhjl = 83; txnhgljhjl > 0; txnhgljhjl--) {
			continue;
		}
	}

}

bool cyxhpmp::myglcbvljn(int axpfbkd) {
	double ghryiwsmx = 21075;
	string dycxfplkzhl = "ewflyfakinhabjuauhmtoxwqisnuyqzwwutdh";
	int czudnjamvagpesc = 4462;
	int ywbymqf = 1100;
	if (1100 != 1100) {
		int yzpmtje;
		for (yzpmtje = 74; yzpmtje > 0; yzpmtje--) {
			continue;
		}
	}
	if (21075 != 21075) {
		int ki;
		for (ki = 57; ki > 0; ki--) {
			continue;
		}
	}
	return false;
}

void cyxhpmp::gswvyjqtssqewxkhwkpzdngxl(bool clptmrgcvzlovh, double lvtavlyzzdh) {
	string crtqkkztpv = "zmsitrszaauoenmjkesflxp";
	bool evaxsrymofcdqv = false;
	int xlwurki = 984;
	string bprncrd = "ayzeljnt";
	double rhkqmimcnbok = 59725;

}

int cyxhpmp::tszixwsrjfrgk(string xsjejj, string xnemwwoyclqlubt, bool xwlykmxh, string drjusgkhyknrht, string tskyvfyhh) {
	double fdmzlvinsv = 34014;
	string aqepc = "cuqjrinjicdxotbmwhejtlziynaqdozwvpyzfuqjasasjtuninoridrikybkjwcjhkcrmuf";
	string ojyvpil = "eswjocmbcbvzgwdsyyaqfohpzfsjmltwt";
	if (string("eswjocmbcbvzgwdsyyaqfohpzfsjmltwt") == string("eswjocmbcbvzgwdsyyaqfohpzfsjmltwt")) {
		int va;
		for (va = 88; va > 0; va--) {
			continue;
		}
	}
	return 35195;
}

int cyxhpmp::qohyqzgokguedzyyujxsjxsgn(bool jdnhzzdvalar, bool lbqtohirdddsvs, int ifrduxipxq, bool hnashunvxjcfpgy, bool ehvfht, bool afbnzplzc, int vgmexste, bool mrdvucls, bool anxvp) {
	return 27494;
}

string cyxhpmp::ciapynczxagkyqqqgcej(double zvefsmnak, bool vrlfmlubmk, double kdxmuiohiakqu, bool hftaxkfiaueab, string qduurdld) {
	return string("qpcugyhfsezvlvthynd");
}

bool cyxhpmp::ayxaptomnefvcwkdlwfczves(string earzs, string srpncww, bool zaunshcixzandy, double bffzw, int ujzbggvqjcf) {
	bool fttlbi = true;
	bool njzxtkgtvr = true;
	double kvevtjsj = 73200;
	string wbrutyyux = "plkwy";
	bool ilicboybjqmkd = true;
	int nqrkfq = 3095;
	return false;
}

string cyxhpmp::pthmbsnzwucryruubzwqzrtm() {
	int vybdofi = 5367;
	bool qaxxsnlixkok = true;
	double htvhgogyemljqtq = 4286;
	int owxdmbfkgkpmca = 979;
	string ztmgymdqnzwfh = "clzelregiigghixulxixhnmkkccucwabryhvvlupabpvfdreqweinzrxqhswpsdnrxktlulchzyjzyzszfjhlfddwtfivjsiccl";
	string qxgsxsahe = "mjdiubsxlmkcmjsqmlkzlnvpukyuqupjgcmuhvchgbtoqhvqfuuonyqlpnwdrnnzdtqsbasigfgdeqqydlrrs";
	if (979 == 979) {
		int jtrz;
		for (jtrz = 23; jtrz > 0; jtrz--) {
			continue;
		}
	}
	if (5367 == 5367) {
		int byrkan;
		for (byrkan = 31; byrkan > 0; byrkan--) {
			continue;
		}
	}
	if (string("clzelregiigghixulxixhnmkkccucwabryhvvlupabpvfdreqweinzrxqhswpsdnrxktlulchzyjzyzszfjhlfddwtfivjsiccl") == string("clzelregiigghixulxixhnmkkccucwabryhvvlupabpvfdreqweinzrxqhswpsdnrxktlulchzyjzyzszfjhlfddwtfivjsiccl")) {
		int hsyvvt;
		for (hsyvvt = 27; hsyvvt > 0; hsyvvt--) {
			continue;
		}
	}
	return string("oyxqnnfvvyytcruqs");
}

string cyxhpmp::teswunuubp(int iqedu, double loqxtynrf) {
	return string("pptkwxvyxnci");
}

int cyxhpmp::gpobuxpbksgkmtaumyq(int vyrgekn, bool yhyyrly, bool wdypnleo, string viqmabwd) {
	int lfgzyufujhckl = 4704;
	int xxrks = 91;
	int jpzxmveri = 6920;
	string nekvteqzbc = "wn";
	bool fbrwwuz = false;
	double efgtuz = 55600;
	string vflyktmt = "aebxlvbxouzteaiaodjgvdezqyojwoepekiemehab";
	bool xwfgsgs = false;
	if (false != false) {
		int ypxcavpn;
		for (ypxcavpn = 41; ypxcavpn > 0; ypxcavpn--) {
			continue;
		}
	}
	if (91 == 91) {
		int auz;
		for (auz = 24; auz > 0; auz--) {
			continue;
		}
	}
	if (string("wn") != string("wn")) {
		int hhsbswshw;
		for (hhsbswshw = 58; hhsbswshw > 0; hhsbswshw--) {
			continue;
		}
	}
	if (false != false) {
		int gpkhbltg;
		for (gpkhbltg = 62; gpkhbltg > 0; gpkhbltg--) {
			continue;
		}
	}
	if (6920 != 6920) {
		int glfvujrorf;
		for (glfvujrorf = 33; glfvujrorf > 0; glfvujrorf--) {
			continue;
		}
	}
	return 35212;
}

string cyxhpmp::dwdwxyyxdtokdhornlolqqgse(int hoyxmhtlwfhu, bool qtoajbkymuju) {
	string ubhqrotvzb = "hnbgosghdelrcrgmgustsbczrcnduymztazboewruvemmucwpjpquoyhcfjomtkqctsvleuwenmstcwphohhuxsrisrkm";
	string cxkqeeepyeyo = "uruuinmgfbjnpowlpyszhefyuohozfgwfkgqgxgduzsljfbcerotjvaexarkrakjz";
	string qylwdlhsaoljaw = "ofqwtptzzpucvyexhhyigappqpddxyubhhtihqaiwzteo";
	double vuaynwwnhkec = 31614;
	bool grfqktqueesusvf = false;
	string vkfoxwjn = "hdnupjyvckyipvbsxewgituxyqjrnhobzcxnvqsloelklpibjvppszdyjhorzbhfqalauojnshxpqowdzidetybqtntg";
	double yimzuzaacckdw = 766;
	double sfuvdcm = 8092;
	string bqnkimbkod = "nkvsusyprczqkwyrafw";
	string ppgdwqierrkz = "ltxyfcfsfnuuhlwveawsnwidulrytxohxokpvecfaiycdxejpybmc";
	return string("urk");
}

cyxhpmp::cyxhpmp() {
	this->myglcbvljn(2051);
	this->gswvyjqtssqewxkhwkpzdngxl(true, 67730);
	this->tszixwsrjfrgk(string(""), string("obrjwmxdfmpjykgzhltelkdxhjkcqyqxxvv"), false, string("zyamgriprkantwduzcljtdxjjwfhydeozqqsvmxtfylomyzwenvmhokfdtxlzkrmctnjyzygrolvwtkdhbjkmnbhnkcx"), string("pkletxruddhvkpbtgogbbpwrhrakazrndcsdptqxlcqiwjonomcjzzbasudbylvwriqfmvvucdnsrkddmdahp"));
	this->qohyqzgokguedzyyujxsjxsgn(false, false, 2731, true, true, true, 5879, true, true);
	this->ciapynczxagkyqqqgcej(33051, true, 7069, false, string("nippduatjkgbduklsiaebzxtwnlddfmbxglyaenhdlqzdfqdojmcz"));
	this->ayxaptomnefvcwkdlwfczves(string("hyrvlccysmpnkezlthmvieabxiipzjgpujhadowzdpwhzdrqrtsbvryse"), string("glumpwakpwlcepgtrgxeuwqidvrzksjygpyeqn"), true, 6854, 3183);
	this->pthmbsnzwucryruubzwqzrtm();
	this->teswunuubp(6951, 32123);
	this->gpobuxpbksgkmtaumyq(6410, false, false, string("ezeuvnkuynhgrxonfgukgkpitxerrbnhedmyrbyfmtfvqglasudtnolaxgnyitokrlbdf"));
	this->dwdwxyyxdtokdhornlolqqgse(1520, false);
	this->jrbbsexsruf(true, 2382, 5721, true, 9411, 1019, 2838, string("cjfloh"), 5055);
	this->nmfknacynnlxbvxl(string("owkmmgvi"), 18198, 81390, 42152, 3081, 1413, 320, 13435, 6819, 3583);
	this->rygfnurcpdve(2324, 4659);
	this->wdxoxlpduynigniumqxmjbcpj(4091, true, string("uvkwdsyflrbzynghaqguhyeljcicomlvfzkfynkcofptxrsmvtklggsgtwbfnkwaawnkeutsimynibmwxkfatonjkvd"), false, 43295, false, true, 7907);
	this->lwozdwgxyi();
	this->esukqbtznzwt(16402, string("vubuihsebckbpzxxewmqiymcdgmiyaefmxfvkjzwwumzimkmehabluwjka"), string("heydbrvtgzuaycgpcyqjsfukexvqkviszxtyezytxzxglqxvusnrpqlwipvuqyzagbzqqotwbxypdxvsfuvlvxdgql"), true, true, 4479);
	this->raalidxpfnxktkl(string("gowdnjzcxxqkvgnnlharugv"), string("cjnaggvmrqzawvxrhbdocnvkqjpfjxlwyqhwkbalwzuafqdit"), false, string("wnsprosn"));
	this->leywlfhyalsuieevmy();
	this->kaxhucmddradcxt(true, 6572, 1555, string("jxisuu"), false);
	this->ibdwwovbxlwwdwsek(string("fjuoqvkosxvxaygdaxtem"), true, string("uzbedszmekpcyzujhapq"), 28150, 19648);
	this->wufrgizxleoohhyqn(49153, 7984, 48637, 19074, 5540);
	this->lyzmudzger(2268, false, 36501, false, true);
	this->neqdwitmwinvzxnq(4889, 4529, 42048, 4322, string("j"), 10807);
	this->qmyrfqnhgapnvukksvcttcqb(1038, 4743, 1245, 230, 33180, false, true, true, 23418, 2212);
	this->htbhiukvsdtogxy(string("nfqezapdmtuwowcseekfoqsustb"), 6375, 22232, 5677, 2261, 59962, 1606, string("kuwbfxxhcwwlddgbrybkpojbakowownodtbnytdzaibqyhzmtrjgjdhuyprgz"), string("fvhsctngaqvieeta"), false);
	this->aynvwlkzsjwlpblnamkro(true, 1903, true, true, string("rmovjynjojedyzpslwrlqliuzugbdcwvmgbhsnhalkpsljfqxnvfmqrjyspesemxyjpaezzmoihjsvyytp"), string("ibthealephlqiyhyzkwcizfnfcddhiefzlryxevjfmfoysrxojiqgbiflxujqjqzudwq"), false, 5433);
	this->xipnjqkwdahikqmtwuns();
	this->nbezbxmuxycwuvd(943, true, string("fhlsgwourwgyoxdceukshrrnpwdteygheqeybhupnjnrpwbl"), 6175, 1186, 1205, 1096, 79608, false, 89);
}

bool CAntiAim::Freestanding(C_BasePlayer* player, CUserCmd *cmd)
{
	if (!g_LocalPlayer || !player || !player->IsAlive() || !g_LocalPlayer->IsAlive())
		return false;

	C_BasePlayer* local = g_LocalPlayer;

	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	static float hold = 0.f;
	Vector besthead;

	auto leyepos = local->m_vecOrigin() + local->m_vecViewOffset();
	auto headpos = local->GetHitboxPos(0); //GetHitboxPosition(local_player, 0);
	auto origin = local->m_vecOrigin();

	auto checkWallThickness = [&](C_BasePlayer * pPlayer, Vector newhead) -> float
	{

		Vector endpos1, endpos2;

		Vector eyepos = pPlayer->m_vecOrigin() + pPlayer->m_vecViewOffset();
		Ray_t ray;
		ray.Init(newhead, eyepos);
		CTraceFilterSkipTwoEntities filter(pPlayer, local);

		trace_t trace1, trace2;
		g_EngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY | MASK_OPAQUE_AND_NPCS, &filter, &trace1);

		if (trace1.DidHit())
		{
			endpos1 = trace1.endpos;
		}
		else
		{
			return 0.f;
		}

		ray.Init(eyepos, newhead);
		g_EngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY | MASK_OPAQUE_AND_NPCS, &filter, &trace2);

		if (trace2.DidHit())
		{
			endpos2 = trace2.endpos;
		}

		float add = newhead.DistTo(eyepos) - leyepos.DistTo(eyepos) + 3.f;
		return endpos1.DistTo(endpos2) + add / 3;

	};

	int index = GetNearestPlayerToCrosshair();
	static C_BasePlayer* entity;

	if (!local->IsAlive())
	{
		hold = 0.f;
	}

	if (index != -1)
	{
		entity = (C_BasePlayer*)g_EntityList->GetClientEntity(index); // maybe?
	}

	if (!entity || entity == nullptr)
	{
		return false;
	}

	float radius = Vector(headpos - origin).Length2D();

	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		float step = (M_PI * 2) / 90;

		for (float besthead = 0; besthead < (M_PI * 2); besthead += step)
		{
			Vector newhead(radius * cos(besthead) + leyepos.x, radius * sin(besthead) + leyepos.y, leyepos.z);
			float totalthickness = 0.f;
			no_active = false;
			totalthickness += checkWallThickness(entity, newhead);

			if (totalthickness > highestthickness)
			{
				hold = besthead;

				highestthickness = totalthickness;

				bestrotation = besthead;
			}
		}
	}

	if (no_active)
	{
		cmd->viewangles.yaw += 180.f;
	}
	else
	{
		cmd->viewangles.yaw = RAD2DEG(bestrotation);
	}

	return false;
}

class dqdhzba {
public:
	bool jsayymfhndb;
	bool xtuas;
	dqdhzba();
	bool kzhchextwhnicyjwpc(int zayxdyfwuvb, string lvkuak, int onemrqc, string rrbvll, int myltakgfrfko, int vcdkpsjfla, bool ghxsqpxy, double mpqodih);
	void ljzkgeienyt(bool aswksvszlwcvoty, bool pfilut, string ptlbrzttyykm, bool gckmvloiegjakz, string fonqfgu, double jlzngv);

protected:
	bool tuxwcciigswtdc;
	double khqem;

	void sibhwrcikpibkoedd(double oldtvbxve);
	string zlywzhwyivlfuniii(bool mogrhdbzlbo, bool ywyllborkrd, int lbuzgrmyxjulrj, int kwjtljb, int bbgex, bool vplkhanc, string qrbgjx);
	void wynlavglbikvbc(double qxwgdclhdf, string vkeogmpwmihdj, bool dndugsyyu, string rmjajrrisagelec);
	bool cgzqupnzoamd();
	void pqbqgegvtrfztiorxgtukoaf(bool fyengvxjdmecj, double kbltggddhgj, int hxqfovqj, int kftscvvixzg, double xrazxkhrv, bool afxckujdbrgews);
	string lycpbpakcoshinvgbsmoletae(string ocnqcx, string ynfeagc);
	void riuwxksjojktt(int njmvollfkcipi, double qaxvkrd, string qrsehirh, int qlxgggtpdmwddys, string vouvirynmmi, double aqeetgjjvvfjttr, double akoej, string smoftplstxrpm);
	bool cifivexzdns(string hjwfftxx, bool wmebjjxqdulhmw, double bhtswlnsygiqy, bool ncidugqeejkymjh, string cwnhmjqkjhljhne, bool cbcpnheiub, bool mfrhtau, double lsbmjazxzfgjq, string qzbfjjuxz, int ffbamhwgkpacqvf);
	int ftwockxjvizbgqbdzd(int gqgov, double thizxyucn, bool mkfoykqi, double msnjec, string lpipsvgtslkbz, int axurpkkmz);

private:
	string vvmge;
	int ltgwjslqoy;

	int qcizleoiazbuummxkkarfh(string gbpfrpyyp, double ijshofklnfyaf, string wnqkpiab, string oiegoi, bool hdxhbeutuo, int udcnbesmplkcnkr, double gxgfrnodnaxn, string ejufwnbipzwm, string jupyuyhvhhxja);
	bool fafzedrwuxnvcbmcoqega(string fknebcqo, double ejksgk, string oougqpams, double yfzku);

};



int dqdhzba::qcizleoiazbuummxkkarfh(string gbpfrpyyp, double ijshofklnfyaf, string wnqkpiab, string oiegoi, bool hdxhbeutuo, int udcnbesmplkcnkr, double gxgfrnodnaxn, string ejufwnbipzwm, string jupyuyhvhhxja) {
	int tfpgargpneenut = 1527;
	int xopadwjlgur = 521;
	string qmtdoi = "hv";
	double gbshrwgqvl = 862;
	double bcuyrhfisrca = 72182;
	bool hozxlaiijpyn = true;
	double zdxjhkuqjqjdum = 6938;
	int mtfiykvr = 9540;
	if (521 == 521) {
		int gxqmuxu;
		for (gxqmuxu = 9; gxqmuxu > 0; gxqmuxu--) {
			continue;
		}
	}
	if (862 != 862) {
		int ggnwn;
		for (ggnwn = 15; ggnwn > 0; ggnwn--) {
			continue;
		}
	}
	if (1527 != 1527) {
		int qqwpcr;
		for (qqwpcr = 2; qqwpcr > 0; qqwpcr--) {
			continue;
		}
	}
	if (string("hv") == string("hv")) {
		int jtjnjliadk;
		for (jtjnjliadk = 25; jtjnjliadk > 0; jtjnjliadk--) {
			continue;
		}
	}
	if (72182 != 72182) {
		int mtfrbfcjtp;
		for (mtfrbfcjtp = 84; mtfrbfcjtp > 0; mtfrbfcjtp--) {
			continue;
		}
	}
	return 61663;
}

bool dqdhzba::fafzedrwuxnvcbmcoqega(string fknebcqo, double ejksgk, string oougqpams, double yfzku) {
	string afxymqbxvuabgui = "ysuxvdiwhsqoztdjczrlrdinichmfmzwbotrkhekoodqchthxuqhpuykyfubtohpiqgeoxjstmf";
	if (string("ysuxvdiwhsqoztdjczrlrdinichmfmzwbotrkhekoodqchthxuqhpuykyfubtohpiqgeoxjstmf") != string("ysuxvdiwhsqoztdjczrlrdinichmfmzwbotrkhekoodqchthxuqhpuykyfubtohpiqgeoxjstmf")) {
		int guiensoplm;
		for (guiensoplm = 53; guiensoplm > 0; guiensoplm--) {
			continue;
		}
	}
	if (string("ysuxvdiwhsqoztdjczrlrdinichmfmzwbotrkhekoodqchthxuqhpuykyfubtohpiqgeoxjstmf") != string("ysuxvdiwhsqoztdjczrlrdinichmfmzwbotrkhekoodqchthxuqhpuykyfubtohpiqgeoxjstmf")) {
		int yuz;
		for (yuz = 28; yuz > 0; yuz--) {
			continue;
		}
	}
	if (string("ysuxvdiwhsqoztdjczrlrdinichmfmzwbotrkhekoodqchthxuqhpuykyfubtohpiqgeoxjstmf") == string("ysuxvdiwhsqoztdjczrlrdinichmfmzwbotrkhekoodqchthxuqhpuykyfubtohpiqgeoxjstmf")) {
		int xlcq;
		for (xlcq = 9; xlcq > 0; xlcq--) {
			continue;
		}
	}
	return false;
}

void dqdhzba::sibhwrcikpibkoedd(double oldtvbxve) {
	bool zpvqltknavyb = false;
	bool whuqcpgdgdvlro = true;
	string ixmoghwpy = "vlwmkcgjcghwupqmjhzlfpgnmxyhtvtouucqpzboueavcqbfonuifhetnsqzznzzhmoyabngfeqqwpuopopqniq";
	string gkrwuce = "uojsdffwnnzcmoztkfhadjwzwvuieurrcjimfadivgsefspnxbriv";
	int sqvvkqhva = 785;

}

string dqdhzba::zlywzhwyivlfuniii(bool mogrhdbzlbo, bool ywyllborkrd, int lbuzgrmyxjulrj, int kwjtljb, int bbgex, bool vplkhanc, string qrbgjx) {
	int nolnvkwqpu = 2791;
	if (2791 != 2791) {
		int qqsiiimgbm;
		for (qqsiiimgbm = 72; qqsiiimgbm > 0; qqsiiimgbm--) {
			continue;
		}
	}
	if (2791 != 2791) {
		int ghgdn;
		for (ghgdn = 72; ghgdn > 0; ghgdn--) {
			continue;
		}
	}
	if (2791 != 2791) {
		int csjtfssv;
		for (csjtfssv = 54; csjtfssv > 0; csjtfssv--) {
			continue;
		}
	}
	return string("");
}

void dqdhzba::wynlavglbikvbc(double qxwgdclhdf, string vkeogmpwmihdj, bool dndugsyyu, string rmjajrrisagelec) {
	double jvrxjdeeiya = 53491;
	bool txrkiicgvpcj = true;
	string aefuhqdpmzxhfq = "korwugwijwmfyck";
	int sxivkirge = 258;
	bool vqjzi = false;
	double onlnlx = 79512;
	bool uiaslb = false;
	bool bhnopo = false;
	bool tthyxrj = true;
	if (string("korwugwijwmfyck") == string("korwugwijwmfyck")) {
		int awg;
		for (awg = 53; awg > 0; awg--) {
			continue;
		}
	}
	if (true == true) {
		int tmwc;
		for (tmwc = 35; tmwc > 0; tmwc--) {
			continue;
		}
	}
	if (true == true) {
		int kylrlkntg;
		for (kylrlkntg = 69; kylrlkntg > 0; kylrlkntg--) {
			continue;
		}
	}
	if (false != false) {
		int uaujqtw;
		for (uaujqtw = 29; uaujqtw > 0; uaujqtw--) {
			continue;
		}
	}
	if (true == true) {
		int iuiwvfzk;
		for (iuiwvfzk = 12; iuiwvfzk > 0; iuiwvfzk--) {
			continue;
		}
	}

}

bool dqdhzba::cgzqupnzoamd() {
	double hqfqvf = 24437;
	double tyrxzmcfwwzrm = 26716;
	int xklqlsto = 856;
	if (24437 != 24437) {
		int qd;
		for (qd = 28; qd > 0; qd--) {
			continue;
		}
	}
	if (856 != 856) {
		int lqdrvhm;
		for (lqdrvhm = 77; lqdrvhm > 0; lqdrvhm--) {
			continue;
		}
	}
	if (26716 == 26716) {
		int xdvxvlg;
		for (xdvxvlg = 27; xdvxvlg > 0; xdvxvlg--) {
			continue;
		}
	}
	if (26716 == 26716) {
		int jpwtaabs;
		for (jpwtaabs = 18; jpwtaabs > 0; jpwtaabs--) {
			continue;
		}
	}
	return false;
}

void dqdhzba::pqbqgegvtrfztiorxgtukoaf(bool fyengvxjdmecj, double kbltggddhgj, int hxqfovqj, int kftscvvixzg, double xrazxkhrv, bool afxckujdbrgews) {
	bool cudkbbzjxxng = false;
	int ugmbecrplmrkde = 4156;
	bool kcwoxbp = false;
	int bppdqnjbknr = 403;
	bool mlrcszpkkm = true;
	string hhxsiiii = "ibwaedzjjrkuwpimdpgctiyidqhbtxugznicswmdfcfuimvuzj";
	bool eckqnsain = true;
	double kforpqa = 2424;
	string zyvcpkktymglt = "gcelttsdeezmyemoemjsstrpjnqakobywlvzqouvmeofmklzyuklgjzeybfvsyfmxpqogvtqswltdbczdxme";
	int upsewqydmn = 2297;
	if (false != false) {
		int exqd;
		for (exqd = 95; exqd > 0; exqd--) {
			continue;
		}
	}
	if (string("ibwaedzjjrkuwpimdpgctiyidqhbtxugznicswmdfcfuimvuzj") != string("ibwaedzjjrkuwpimdpgctiyidqhbtxugznicswmdfcfuimvuzj")) {
		int pplgkoesa;
		for (pplgkoesa = 4; pplgkoesa > 0; pplgkoesa--) {
			continue;
		}
	}
	if (false == false) {
		int rliipf;
		for (rliipf = 69; rliipf > 0; rliipf--) {
			continue;
		}
	}
	if (2424 == 2424) {
		int mztjqhs;
		for (mztjqhs = 13; mztjqhs > 0; mztjqhs--) {
			continue;
		}
	}
	if (4156 != 4156) {
		int xpzvcxc;
		for (xpzvcxc = 25; xpzvcxc > 0; xpzvcxc--) {
			continue;
		}
	}

}

string dqdhzba::lycpbpakcoshinvgbsmoletae(string ocnqcx, string ynfeagc) {
	string sbxkx = "dunennbuzfstmwmqctrxoiiehvrwwmrbhymgustdpnzjisyqdyqwdahbcoinzmvzdomrzwul";
	int lrwrcc = 552;
	string lakwqdipbzboa = "sswseieyjjkdqzfjtatskorfjdoasvaai";
	if (string("sswseieyjjkdqzfjtatskorfjdoasvaai") == string("sswseieyjjkdqzfjtatskorfjdoasvaai")) {
		int vwydp;
		for (vwydp = 95; vwydp > 0; vwydp--) {
			continue;
		}
	}
	if (string("sswseieyjjkdqzfjtatskorfjdoasvaai") == string("sswseieyjjkdqzfjtatskorfjdoasvaai")) {
		int cymap;
		for (cymap = 39; cymap > 0; cymap--) {
			continue;
		}
	}
	if (string("sswseieyjjkdqzfjtatskorfjdoasvaai") != string("sswseieyjjkdqzfjtatskorfjdoasvaai")) {
		int mrijldnnc;
		for (mrijldnnc = 60; mrijldnnc > 0; mrijldnnc--) {
			continue;
		}
	}
	if (string("sswseieyjjkdqzfjtatskorfjdoasvaai") != string("sswseieyjjkdqzfjtatskorfjdoasvaai")) {
		int phqtwv;
		for (phqtwv = 74; phqtwv > 0; phqtwv--) {
			continue;
		}
	}
	return string("hfa");
}

void dqdhzba::riuwxksjojktt(int njmvollfkcipi, double qaxvkrd, string qrsehirh, int qlxgggtpdmwddys, string vouvirynmmi, double aqeetgjjvvfjttr, double akoej, string smoftplstxrpm) {
	int qiyeywzrmjjlp = 982;
	bool ctooygrlkmeip = false;
	int wlobkhdsm = 1007;
	int lvvxalvrat = 7636;
	int xenciczcuvksa = 4421;
	int skqdar = 974;
	int pyrtscfhrztvh = 816;
	double dipqliznl = 38062;
	string eqtahncajp = "gocljxjdgdpgdrcutgcjxanwppkphnklwxvrdtzwcroimcvcnlwmdrzptkxpd";
	double ssvceawekzrnhu = 31444;
	if (false == false) {
		int su;
		for (su = 43; su > 0; su--) {
			continue;
		}
	}
	if (string("gocljxjdgdpgdrcutgcjxanwppkphnklwxvrdtzwcroimcvcnlwmdrzptkxpd") != string("gocljxjdgdpgdrcutgcjxanwppkphnklwxvrdtzwcroimcvcnlwmdrzptkxpd")) {
		int cmdccfp;
		for (cmdccfp = 42; cmdccfp > 0; cmdccfp--) {
			continue;
		}
	}
	if (974 != 974) {
		int owrewy;
		for (owrewy = 48; owrewy > 0; owrewy--) {
			continue;
		}
	}

}

bool dqdhzba::cifivexzdns(string hjwfftxx, bool wmebjjxqdulhmw, double bhtswlnsygiqy, bool ncidugqeejkymjh, string cwnhmjqkjhljhne, bool cbcpnheiub, bool mfrhtau, double lsbmjazxzfgjq, string qzbfjjuxz, int ffbamhwgkpacqvf) {
	string sorjyg = "xsqsidmsedjdkabknnrioxhzoxpohpkvhc";
	bool fbmspsrma = true;
	double fqdeexravzcmhl = 32466;
	bool rcwwzsellthy = true;
	double kzdqctdl = 12155;
	string dsspetsyz = "hjiamorwuzbcjjodzqzehflxlfoixpvazpre";
	if (32466 != 32466) {
		int qp;
		for (qp = 90; qp > 0; qp--) {
			continue;
		}
	}
	return true;
}

int dqdhzba::ftwockxjvizbgqbdzd(int gqgov, double thizxyucn, bool mkfoykqi, double msnjec, string lpipsvgtslkbz, int axurpkkmz) {
	double xejtvwhwhdcbf = 27915;
	bool yxxpuydfvec = false;
	double pwnwqmhemeaidx = 39833;
	string fodzho = "ah";
	bool vsmbuoeqxzpp = true;
	int kqwxs = 2293;
	double hcwmc = 58067;
	bool odgrz = true;
	if (true == true) {
		int fawkwzgcb;
		for (fawkwzgcb = 20; fawkwzgcb > 0; fawkwzgcb--) {
			continue;
		}
	}
	if (string("ah") != string("ah")) {
		int sdnjckjtk;
		for (sdnjckjtk = 66; sdnjckjtk > 0; sdnjckjtk--) {
			continue;
		}
	}
	return 1198;
}

bool dqdhzba::kzhchextwhnicyjwpc(int zayxdyfwuvb, string lvkuak, int onemrqc, string rrbvll, int myltakgfrfko, int vcdkpsjfla, bool ghxsqpxy, double mpqodih) {
	return true;
}

void dqdhzba::ljzkgeienyt(bool aswksvszlwcvoty, bool pfilut, string ptlbrzttyykm, bool gckmvloiegjakz, string fonqfgu, double jlzngv) {
	bool nsmcqbadimwyzn = true;
	string hkfrzzfjwmauiq = "yftmtpgcednkoeeqzjzccuwoehc";
	int gljecbbdz = 2265;
	string sznlvnb = "hhxqjhrnjnrvjkkdfvlwxtbyktdcbnmxmpoilwmx";
	int snhce = 9021;
	bool owgcgjuryllynij = true;
	bool tpyuiuf = true;
	if (string("hhxqjhrnjnrvjkkdfvlwxtbyktdcbnmxmpoilwmx") != string("hhxqjhrnjnrvjkkdfvlwxtbyktdcbnmxmpoilwmx")) {
		int xuy;
		for (xuy = 70; xuy > 0; xuy--) {
			continue;
		}
	}
	if (9021 == 9021) {
		int me;
		for (me = 60; me > 0; me--) {
			continue;
		}
	}

}

dqdhzba::dqdhzba() {
	this->kzhchextwhnicyjwpc(1818, string("ilmhteawhmhngztptqqozgjltmusxvyfzhozvijsxzubiarbawmocssbmlnfrp"), 515, string("hqiluvnjbjnvjvgrbndymntwajbwdwyqvxoregoafkgvofmwkjlzwacnaapdcdbhfsycbxomdstjpapaomdrx"), 419, 8839, false, 44313);
	this->ljzkgeienyt(true, false, string("rtnxarxvuadqitwlpbyrxpohymuqjbjtnegmwhdteamxdcqobmsnjhnxoymeehqnfxoqfdhmzivyjscvhks"), false, string("apwzuadgdyrnyihigltyikbwgxzpvjwbxxhlgkvogqtxntgfiqvwfmyj"), 39738);
	this->sibhwrcikpibkoedd(1916);
	this->zlywzhwyivlfuniii(false, false, 343, 2113, 2955, false, string("yulkrochrtjggdcmxchzrqom"));
	this->wynlavglbikvbc(21361, string("anxyodqbmjzauqowfbjdsprqwqhuybycezshfvzvpxyqwjuclv"), false, string("pufrtavzriiviqazgqu"));
	this->cgzqupnzoamd();
	this->pqbqgegvtrfztiorxgtukoaf(true, 44143, 3065, 136, 13359, false);
	this->lycpbpakcoshinvgbsmoletae(string("iescgglvehbaxcjbjsvdfukasrgxaaqoexiasjnttvfweitulmgg"), string("rjqnftmqmmkzotoyexpjhfmcmeeuberlpscbtakpnjukpzjm"));
	this->riuwxksjojktt(3797, 5018, string("tpwwnphctoqwcrdvueyrvrfmnrqvvaezomeanavdixqbwuehnnvkkgrmslqzqclrhyycunyjzcb"), 2244, string("wuccoczhbjuwgdxdxgfeblrauxczlkknedsuiygttlfdmajrmmckj"), 5060, 60104, string("fhrhubkilpmbxidmxoaskbgzkfzfiduzrwgavwrqtqoqosdzrjrtpwitzpxcmrgtsmuixvccycbtqwquhltdafhsttqdlb"));
	this->cifivexzdns(string("pssbxsnjwjmnnepweqdixheygjvemgjntxdecvqmyaawhrwkvvjiuyncqcvllwfumrneeqxfvrpbbfucocpdfnrsxmjz"), true, 58631, false, string("bclmqbohuopsnvz"), true, true, 474, string("vikr"), 3781);
	this->ftwockxjvizbgqbdzd(1520, 12301, false, 20782, string("nobedhqkunjfthgajolqpcgskympyoagkumqivnfnmobvwttocxnrirdwbrhbzsamcq"), 3622);
	this->qcizleoiazbuummxkkarfh(string("ltgzektpyarsbwvagxqcwgkrihjhcgzsktzibsgphtlgjmizivjgfyayhcgbmgaazossllessmfikyf"), 2013, string("hocgoyrxogtaizmfezgcfmywhhdtlqmhk"), string("jjjayfsogtzdkewhezzepjuzymbvqulyggoscxdqxyivvqttmkfuzikbdbiodrrjdpk"), true, 7183, 4695, string("xrdfxhpmiuenvrhkmfhoufbiaspbbymowf"), string("gjuaguisnzuotdzvgadisxycwtrscbidplybgraguvqcoamlauegigjkkjuecopzdjfkgfhfssn"));
	this->fafzedrwuxnvcbmcoqega(string("kcsukifaxiuqyfzstomrsfnidxwupcqnworoyvajskxrcngtizijzslnkoaqkobzowxgkpqjrntkjafsgumziriqwincxarop"), 2849, string("bsxpevtszcjatvydrnimthjnndacytuyy"), 4772);
}

int CAntiAim::GetNearestPlayerToCrosshair()
{
	float BestFov = FLT_MAX;
	int BestEnt = -1;
	QAngle MyAng;
	g_EngineClient->GetViewAngles(MyAng);

	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!entity || !g_LocalPlayer || !entity->IsPlayer() || entity == g_LocalPlayer || entity->IsDormant()
			|| !entity->IsAlive() || entity->IsTeammate())
		{
			continue;
		}

		float CFov = fov_player(g_LocalPlayer->m_vecOrigin(), MyAng, entity); //Math::GetFOV(MyAng, Math::CalcAngle(g_LocalPlayer->GetEyePos(), entity->GetEyePos()));

		if (CFov < BestFov)
		{
			BestFov = CFov;
			BestEnt = i;
		}
	}

	return BestEnt;
}

void NormalizeNum(Vector& vIn, Vector& vOut)
{
	float flLen = vIn.Length();

	if (flLen == 0)
	{
		vOut.Init(0, 0, 1);
		return;
	}

	flLen = 1 / flLen;
	vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class omnofrq {
public:
	int vtrdkchlqqqgsz;
	int mjbzktqnxt;
	bool lwbgh;
	double aeqof;
	omnofrq();
	double ejsioullqispjjneupkevlwy(double uxpfvxajkczxjsg, string cirdoxvvndgdu, string uqohdhxgkjb, double cbkiagev, double kkldykg, bool eporaxoaqrrp, string gvymv);
	void mlxdqlxfoccgcsohajlorhbho(string kpjtnyf);
	int cofqokeyvtfuklmcldz(string gpaqpo, bool bycmpxg, bool fkyiyn, string ajylb, string cmjqnwhdntltyp, bool buousw, double jojxwttkkgx, int fefvmzcdjrb, double zdjpskuqiom);
	int bghgltycilwufshq(int mtrzcz, string achaccuksyxgs, bool cbjexjaujrc, double tpoheacqdvubic);

protected:
	double llvsbfitvr;
	bool reepwfhgbt;

	string tnrnilrlivrb(double udvkljvndahrgd, int sabskfzs, bool lypyuqkeiuurwpw, string qcwdbv, int bgwpqfak, bool thkyhiirjgz);
	string clbdzgqjhtvwagpmim(bool sossdlfz, bool nuudc, int ztuguft, bool hvkasrfp);
	bool vpotvpxlhy(int xaxnqsuomjnjpp, bool amrjsdms);
	string kirpeavwll(string fiuhkufjecmrv, int bvlqwngznp, double bqcjzjqaeq, bool ogrrxczj, int qfvbsyodqyztx, int nsbgaxjumx, double elkupwndwiudgxd, int ugjjqucyxgqygua, int vkacrvxylix);
	void wihjqcqxzynghzoyuhsyxhlqo(string fumig, double bqncqvqnnl, string jpylmlkoditgh, string nkrigppbonvoeho, bool ihkaporfxbrdrk, bool plixu, string ncadmdwqcqefr, int bbzlkcyylaosaj);
	void meaipllscqho(double qbqjrzsjbhcsq, string xxgdiomxyhnxux, string avirhe, double xmrxqwrqbmrrm, int luirzooafb);
	string shopjwktwsulcinqsiuwhmf(int rpfinyzhyyocwy, double hidqjzkuggykny, string tvhqskrawxcenjq, double kstyhfbwyqmvc, int znmeuuyjclk, string etwmigwnzlggl);

private:
	bool dhdcryfxwmel;
	bool dulvovjlcgyrogd;
	string khvhitgzprxaw;

	bool lljbtpfirazpretkjkddf(string grnxhw, double euhtgrpeucok);

};



bool omnofrq::lljbtpfirazpretkjkddf(string grnxhw, double euhtgrpeucok) {
	bool ryuwlybygsfukt = true;
	int jnflqrnjecdlov = 5203;
	int sdypvpycjtmua = 1703;
	string afkodzii = "yyarutzmgtfvpfbvefhdynzvjzjlhfkpwtcmnszzentuqpzfybexyaltxyjcjwdfneguxneakexhianajhqsmcdogldemf";
	int naizyskiwigb = 6034;
	double xfhqvkwzbthybi = 1675;
	bool btbsex = false;
	double tkdvovhxijjtoq = 90733;
	string hajkrdxitdt = "dxjxrcysporajtaeqduhbdltsqcjyhxagmqdyblfkyi";
	return true;
}

string omnofrq::tnrnilrlivrb(double udvkljvndahrgd, int sabskfzs, bool lypyuqkeiuurwpw, string qcwdbv, int bgwpqfak, bool thkyhiirjgz) {
	double cxsnrxufed = 14325;
	double hgkboomiy = 514;
	int fuuwgd = 3969;
	double csllkyo = 1407;
	int htkfjpjhbrphjk = 244;
	bool evwqppmxkkwzqq = false;
	return string("agfuncdoqaabezwe");
}

string omnofrq::clbdzgqjhtvwagpmim(bool sossdlfz, bool nuudc, int ztuguft, bool hvkasrfp) {
	string rowplahiivkx = "uaacwpblnaxdzgmupfqofmjfbfdroxtvptvwvumqdkhiajlbqppaslrjxgmiwjtrdyoyumims";
	double cmrewiygkqnodki = 32145;
	bool suqfdembbr = true;
	string oevgrpp = "ftcbgxhgppoatlpfnyxlxlukzikcmg";
	if (string("ftcbgxhgppoatlpfnyxlxlukzikcmg") == string("ftcbgxhgppoatlpfnyxlxlukzikcmg")) {
		int ommnyy;
		for (ommnyy = 83; ommnyy > 0; ommnyy--) {
			continue;
		}
	}
	return string("mmxuqpnpeghkkqozohm");
}

bool omnofrq::vpotvpxlhy(int xaxnqsuomjnjpp, bool amrjsdms) {
	bool rpvlctwonwu = true;
	int azfdwanpfk = 461;
	string esjavbtnuxd = "meobubmhfvcsnveekyzirpniodlqkgqguxdfujgufemnrozwyrbznyntctgfcnmbtiuitxooqowtatshzprhy";
	string twesazhtuf = "tnvkmktookmpxjgjeonbxfqscgghpiahqsfuvcwjwbhgiylnnuhhfehkzmwmzwgqebxqgkdpodleuszfaknbkxlyxzlzh";
	double hpbacdhgu = 67078;
	string wahkzpa = "jtwdnpoefkokwejdgnfvlonqwejhaecthps";
	string svwalttcgpwupy = "fotnjcczigkaaiwcipcvmunhjelfgyzyctabfr";
	int zdymexbtqos = 357;
	bool xtbvryjwmroohf = false;
	double gtyrpxg = 41847;
	if (string("meobubmhfvcsnveekyzirpniodlqkgqguxdfujgufemnrozwyrbznyntctgfcnmbtiuitxooqowtatshzprhy") != string("meobubmhfvcsnveekyzirpniodlqkgqguxdfujgufemnrozwyrbznyntctgfcnmbtiuitxooqowtatshzprhy")) {
		int bhpuqb;
		for (bhpuqb = 47; bhpuqb > 0; bhpuqb--) {
			continue;
		}
	}
	if (false == false) {
		int yppkcyp;
		for (yppkcyp = 63; yppkcyp > 0; yppkcyp--) {
			continue;
		}
	}
	if (461 != 461) {
		int kzabzpmd;
		for (kzabzpmd = 64; kzabzpmd > 0; kzabzpmd--) {
			continue;
		}
	}
	if (string("tnvkmktookmpxjgjeonbxfqscgghpiahqsfuvcwjwbhgiylnnuhhfehkzmwmzwgqebxqgkdpodleuszfaknbkxlyxzlzh") != string("tnvkmktookmpxjgjeonbxfqscgghpiahqsfuvcwjwbhgiylnnuhhfehkzmwmzwgqebxqgkdpodleuszfaknbkxlyxzlzh")) {
		int egyjdv;
		for (egyjdv = 65; egyjdv > 0; egyjdv--) {
			continue;
		}
	}
	if (67078 == 67078) {
		int ugdwcel;
		for (ugdwcel = 42; ugdwcel > 0; ugdwcel--) {
			continue;
		}
	}
	return true;
}

string omnofrq::kirpeavwll(string fiuhkufjecmrv, int bvlqwngznp, double bqcjzjqaeq, bool ogrrxczj, int qfvbsyodqyztx, int nsbgaxjumx, double elkupwndwiudgxd, int ugjjqucyxgqygua, int vkacrvxylix) {
	int bblsauq = 429;
	string zhcfhdnrtux = "nhnqholyzzlywztbnmkynduumiumkpudwyie";
	bool xwxdygnqwlqs = true;
	string wysbuxjemhix = "xesvxoklxxahlahtbmbujqq";
	int sngrx = 1973;
	if (string("nhnqholyzzlywztbnmkynduumiumkpudwyie") != string("nhnqholyzzlywztbnmkynduumiumkpudwyie")) {
		int sd;
		for (sd = 11; sd > 0; sd--) {
			continue;
		}
	}
	if (429 == 429) {
		int wa;
		for (wa = 53; wa > 0; wa--) {
			continue;
		}
	}
	return string("");
}

void omnofrq::wihjqcqxzynghzoyuhsyxhlqo(string fumig, double bqncqvqnnl, string jpylmlkoditgh, string nkrigppbonvoeho, bool ihkaporfxbrdrk, bool plixu, string ncadmdwqcqefr, int bbzlkcyylaosaj) {
	double gjkhhp = 50962;
	string zidiwganmptl = "rdbuujemvauwyuaxvgjy";
	string xnzmb = "qqysytshrzudchvntpt";
	string vpbrfpjcdr = "vjbyxbpiibhbtvizecysbfvpbhigpmaegiyuqapphzlcycsfmcylparibaglabkhqbdtvtqrjnlhbxgrqbuehozz";
	int fhohshomxh = 2415;
	bool beoummecqjv = false;
	bool ovjsyelf = false;
	int vwvgnzvyqiub = 534;
	string khgxoyn = "ovlhoxyxgzjrdgnrfeqwdocvcbrhwwykoitgoreukdwfuafwwecptvwt";
	if (false != false) {
		int aei;
		for (aei = 95; aei > 0; aei--) {
			continue;
		}
	}
	if (string("ovlhoxyxgzjrdgnrfeqwdocvcbrhwwykoitgoreukdwfuafwwecptvwt") == string("ovlhoxyxgzjrdgnrfeqwdocvcbrhwwykoitgoreukdwfuafwwecptvwt")) {
		int gbsmzka;
		for (gbsmzka = 88; gbsmzka > 0; gbsmzka--) {
			continue;
		}
	}

}

void omnofrq::meaipllscqho(double qbqjrzsjbhcsq, string xxgdiomxyhnxux, string avirhe, double xmrxqwrqbmrrm, int luirzooafb) {
	double qvcyiukfcymz = 47083;
	int ryvzcbxo = 1860;
	double xtmwdmppevqbma = 66169;
	double ukzok = 29779;
	bool jcjru = false;
	int wrlgwl = 2429;
	string ixsojsn = "qfvrqztsnhkkehqpijgihfyvfddmhcbipghsbeandjrjiwopouhuiytjpqtoxvvbctkpiygeg";
	double jhwssprkhu = 798;
	bool rqokjwxhrpvb = true;
	if (47083 == 47083) {
		int uyabele;
		for (uyabele = 79; uyabele > 0; uyabele--) {
			continue;
		}
	}

}

string omnofrq::shopjwktwsulcinqsiuwhmf(int rpfinyzhyyocwy, double hidqjzkuggykny, string tvhqskrawxcenjq, double kstyhfbwyqmvc, int znmeuuyjclk, string etwmigwnzlggl) {
	int wxyfciopbmk = 2275;
	bool cqsiimhrf = true;
	int xodnigiu = 3157;
	bool haxftuqi = false;
	bool grhpzebptl = true;
	string jfzbhsdamy = "pclgpwrxwxcptlobwhqviirklvkycyjnlbikanpnnmfkpkqkxtllaifmplkdswhsqbfpmywlagseswvkabdzmxdanrgooazlubaj";
	int yddpmvgg = 672;
	if (true == true) {
		int apqp;
		for (apqp = 53; apqp > 0; apqp--) {
			continue;
		}
	}
	if (false != false) {
		int hvabeiqg;
		for (hvabeiqg = 10; hvabeiqg > 0; hvabeiqg--) {
			continue;
		}
	}
	if (string("pclgpwrxwxcptlobwhqviirklvkycyjnlbikanpnnmfkpkqkxtllaifmplkdswhsqbfpmywlagseswvkabdzmxdanrgooazlubaj") == string("pclgpwrxwxcptlobwhqviirklvkycyjnlbikanpnnmfkpkqkxtllaifmplkdswhsqbfpmywlagseswvkabdzmxdanrgooazlubaj")) {
		int kbna;
		for (kbna = 4; kbna > 0; kbna--) {
			continue;
		}
	}
	if (string("pclgpwrxwxcptlobwhqviirklvkycyjnlbikanpnnmfkpkqkxtllaifmplkdswhsqbfpmywlagseswvkabdzmxdanrgooazlubaj") != string("pclgpwrxwxcptlobwhqviirklvkycyjnlbikanpnnmfkpkqkxtllaifmplkdswhsqbfpmywlagseswvkabdzmxdanrgooazlubaj")) {
		int xzdubumyjv;
		for (xzdubumyjv = 12; xzdubumyjv > 0; xzdubumyjv--) {
			continue;
		}
	}
	if (false != false) {
		int oe;
		for (oe = 36; oe > 0; oe--) {
			continue;
		}
	}
	return string("otfgc");
}

double omnofrq::ejsioullqispjjneupkevlwy(double uxpfvxajkczxjsg, string cirdoxvvndgdu, string uqohdhxgkjb, double cbkiagev, double kkldykg, bool eporaxoaqrrp, string gvymv) {
	bool mjbdwr = true;
	bool ltltdecpuz = false;
	string nfddc = "mguepzgrmnwgtxwronstvicehdwvghqkmgeytvjjvzovwfbvwkrislnllvalqekcappcgydzvavdeozd";
	if (false != false) {
		int uounxpw;
		for (uounxpw = 75; uounxpw > 0; uounxpw--) {
			continue;
		}
	}
	if (true != true) {
		int limai;
		for (limai = 36; limai > 0; limai--) {
			continue;
		}
	}
	return 33092;
}

void omnofrq::mlxdqlxfoccgcsohajlorhbho(string kpjtnyf) {
	bool chbargzozjxus = true;
	bool ksfrjwqhf = false;
	string slsdivdjav = "mtwnexroaakqwysnxlhhwxtpiizoulxnwwedpirwunoqahtqvpyprgyuikbjetkbqlzaebgqikzvllneviids";
	bool vldyj = true;
	double unueemyffjnlah = 4432;
	int bzwibohb = 66;
	if (false != false) {
		int mlcfbg;
		for (mlcfbg = 70; mlcfbg > 0; mlcfbg--) {
			continue;
		}
	}

}

int omnofrq::cofqokeyvtfuklmcldz(string gpaqpo, bool bycmpxg, bool fkyiyn, string ajylb, string cmjqnwhdntltyp, bool buousw, double jojxwttkkgx, int fefvmzcdjrb, double zdjpskuqiom) {
	int ezrvrilyncex = 1826;
	bool eesvtppqwzvr = false;
	double qbebnnnwbczfp = 15491;
	double qrnolyibgrckksr = 40864;
	bool oingdhmmqjsl = false;
	if (40864 != 40864) {
		int ca;
		for (ca = 36; ca > 0; ca--) {
			continue;
		}
	}
	if (40864 == 40864) {
		int pqfvw;
		for (pqfvw = 50; pqfvw > 0; pqfvw--) {
			continue;
		}
	}
	return 16935;
}

int omnofrq::bghgltycilwufshq(int mtrzcz, string achaccuksyxgs, bool cbjexjaujrc, double tpoheacqdvubic) {
	int aiypmrc = 2701;
	double kycjcfteofrd = 6625;
	bool rixttdhpfxx = true;
	bool vlvzprmkdem = true;
	if (2701 != 2701) {
		int bzqtxc;
		for (bzqtxc = 7; bzqtxc > 0; bzqtxc--) {
			continue;
		}
	}
	if (true != true) {
		int emi;
		for (emi = 25; emi > 0; emi--) {
			continue;
		}
	}
	if (true != true) {
		int qujryrpcr;
		for (qujryrpcr = 91; qujryrpcr > 0; qujryrpcr--) {
			continue;
		}
	}
	if (true != true) {
		int uboes;
		for (uboes = 20; uboes > 0; uboes--) {
			continue;
		}
	}
	if (6625 != 6625) {
		int wuk;
		for (wuk = 51; wuk > 0; wuk--) {
			continue;
		}
	}
	return 572;
}

omnofrq::omnofrq() {
	this->ejsioullqispjjneupkevlwy(2068, string("eqfkwxjvijxmldkzdgektswqebtvsodawirwdpfldtiwhownxmvouvhojlljlmawubwthfhjlyfjewecigzpeop"), string("biagnzqlqigljlhjcdusvaolhtdvqozticzjyxifdialtloqfmjemhgwghvizawog"), 16811, 67262, true, string("yqaoodlxsiddufocudzwvnmcboealqafdxodi"));
	this->mlxdqlxfoccgcsohajlorhbho(string("gakdezzhzufocbbitmrrydcxhhoiguwlvcrnbamrxjdxqgyuvkslpclfntshedxczsajbtuddkxzmu"));
	this->cofqokeyvtfuklmcldz(string("jvisik"), false, true, string("vsleecxzcxsryrznegcddedtpczrqvbtuyiuzbffidnhaidnkbvnqmwdrfmkuhgimasylgenpukmfljsjpznpx"), string("hxcscdmadjhxlfvddtkunwsvgesmciwmkuizrfjobxdxcoyfvkpjpcwrdpwbmts"), true, 7921, 7720, 51030);
	this->bghgltycilwufshq(3572, string("iudflfodgbjzcwhlxfbjedajlizxpihzklgmhnkcbutkbssfddadcksuienusszvzedwejuwzacsmjmvdwigqmlvsjh"), true, 7193);
	this->tnrnilrlivrb(11036, 2255, true, string("alabscrberrqyuktltqbogggawmiqglqrwgbotdupchvxfcbbjaaqcyzvrectbmlcmztpdwbrv"), 1370, true);
	this->clbdzgqjhtvwagpmim(true, false, 743, true);
	this->vpotvpxlhy(6522, true);
	this->kirpeavwll(string("exjuzwfqzweczuajxoelgvbbgzulsxglxzzgfmdsmfibhrviqhgzpkmzdyoqb"), 1599, 69834, true, 1928, 3149, 4463, 5986, 1720);
	this->wihjqcqxzynghzoyuhsyxhlqo(string("jroknhmrtoflnmmdlbhoqlrdlupeiixppfvhjqzkkdpimawryjdnsj"), 17515, string("jbuysxikncyulhdpqkyjptlldlckkjfgyrzeihjqilekxdihbeiekgbppzbklnqnveymhckh"), string("ucdgkqxjhkgucavjacdtkaidunpcplogojwemtgsmdnuudhkfgkalqnnidhhecwrujtwzkhtrrvdqxyhfjvllnlzws"), false, true, string("xjywvwhufetvblqsjqmrxvhgigeaocjwdjgwkiixbvfexe"), 5487);
	this->meaipllscqho(28304, string("fuahcqmxpaqcqnmbxjyoslsgdexxub"), string("gpiglwegmtqkgvecjqfkscpaqrwspkeffktbxevq"), 60899, 552);
	this->shopjwktwsulcinqsiuwhmf(104, 24494, string("poimuvxcehnxklflfpwbiuvbyrxfkxgvliahmzdxobzvaimdlwyseudbodytw"), 12010, 1409, string("kmsctkxibvxjwlnlhiycbdandjlv"));
	this->lljbtpfirazpretkjkddf(string("fryllvbtwf"), 39363);
}



float CAntiAim::fov_player(Vector ViewOffSet, QAngle View, C_BasePlayer* entity)
{
	// Anything past 180 degrees is just going to wrap around
	CONST FLOAT MaxDegrees = 180.0f;

	// Get local angles
	QAngle Angles = View;

	// Get local view / eye position
	Vector Origin = ViewOffSet;

	// Create and intiialize vectors for calculations below
	Vector Delta(0, 0, 0);
	//Vector Origin(0, 0, 0);
	Vector Forward(0, 0, 0);

	// Convert angles to normalized directional forward vector
	Math::AngleVectors(Angles, Forward);

	Vector AimPos = entity->GetHitboxPos(HITBOX_HEAD); //pvs fix disabled

													   //VectorSubtract(AimPos, Origin, Delta);
	Origin.VectorSubtract(AimPos, Origin, Delta);
	//Delta = AimPos - Origin;

	// Normalize our delta vector
	NormalizeNum(Delta, Delta);

	// Get dot product between delta position and directional forward vectors
	FLOAT DotProduct = Forward.Dot(Delta);

	// Time to calculate the field of view
	return (acos(DotProduct) * (MaxDegrees / M_PI));
}


















class ysqywus {
public:
	bool glhvdm;
	string xonuvjsihuihj;
	int zyeyqedxgmiqm;
	int wzuqupjhlkviqi;
	ysqywus();
	double gticcwciliiwgdomla(bool sfurhvywbc, bool rzwehxb, double bqtmjh);

protected:
	int bieomcw;
	string wugivnapzs;
	double oafvhtj;
	bool ptoxmcglhqdnrr;
	double qnnze;

	int czueqdzmklkwotcbdghmln(double vnfoikkypxtfc);
	void panibaqzwcjerbwnbif(double kpbehlhxowtchag, double drxdzjavcj, double sjpseqppxqcazww, bool hadqu, double mtfibsyktqu);
	void dvhjdensorutgazugz(string iyryaxiibdozkw, bool naeobphfo, double hbopbifgqvbpe, bool tskkpczb, double lwsoqkuhip, bool qkhtvgabjmvfwsz, int vsoww, int atkwphom, string jkvatp);
	bool adodrtazodbbejnogwrhpg(double fodsupkrphmf, string kzshkxqb, int xwmpkboiyriymjp, string dbahsdz, double ekkyjmjos, bool wrthd, bool wwwpaddynjyby, string vcqqxqs, int kxgnqeyyz, bool clumjjbjybro);
	bool shxwiwvgdztuktkpz(int lnakbos, double cvroqzfp, int ydaansz, string yuznglvtq, bool fhztxfos, double lfiysh, string uvferivp);
	double bknbalatafualhn(bool wuocgpgfytgmd);
	int zpujbbcfjlidprwtlmztvkgq(bool dfeiunjpiuaneo, double exnpusmmj, double xdckhjxvrvumwun, double qisdwgd);

private:
	double redllwxiet;
	double rremxyuaka;
	bool ccymduhprku;
	int pwdzatwfqy;
	string traivgroksa;

	void pwekbcjknzg(int hyabkznh, int kmgdnimhpkus, int hmyfiwqzldrhmea, int eqoznstyhr);
	string uyspazmwbimzhbyxnktb(int omnxjrfdpqg, int tqdoevokkhg, double ruibvoeqgr, double wmfwlapv);
	double mzfcgtvkbvxisimqqk(double hsoeahfarh, double rdxvl, string ppkuawq, double uikonqlgwjefjof, bool nspvnme, double gbekkwmrsgyzsr, int cxrnc, int tfzinnruwarfw, string nfxtywlgdokahx, string lmjidrcwq);
	bool noasagtajmwdfxwfo(string dwpmxcmqyre);
	int hiwmshxcursyfvmoawluvhguo(int usqkyjx, bool upwckvwjymbbcox);
	double aqsmeurlehcingufvbpsh(string nyfximhxjjdnat, bool xkiyp, int wmjoisin, string mpsoakggdso, bool cllcwnf, double cnwpt, string ahoklesjfycmei, string faiqiasojcetw);
	bool jzsikcqdengnwrqcusf(bool iaqnbvsiaepydgm, int hwjbv, double rmgfzjbsslj, int ihkudo, string hhkdjobpgv, bool kjicbofdu, int ikasyeyocoznfaa, double xjnmfjpot, double cthohwepr);
	double wychrkyipznpdo(string xavcifwpug, string ktgoswn, string abededvngsahf);
	bool orazlywsnrernwhgrhgobv(bool sfbclbpyqr, bool qqpngbf, double wuerthwahdwk, int bwmgguqslbjf, bool jiaxrddpkbfzxf, int bjihmxhlyn, int espakgeui, int lqcfz);
	bool shtemorqxhiaaxpdoof(double gapennjgculsa);

};



void ysqywus::pwekbcjknzg(int hyabkznh, int kmgdnimhpkus, int hmyfiwqzldrhmea, int eqoznstyhr) {
	int fecogqfaekmsra = 3191;
	if (3191 != 3191) {
		int sgb;
		for (sgb = 4; sgb > 0; sgb--) {
			continue;
		}
	}
	if (3191 != 3191) {
		int ojloi;
		for (ojloi = 78; ojloi > 0; ojloi--) {
			continue;
		}
	}
	if (3191 == 3191) {
		int cxg;
		for (cxg = 41; cxg > 0; cxg--) {
			continue;
		}
	}
	if (3191 == 3191) {
		int abqwds;
		for (abqwds = 4; abqwds > 0; abqwds--) {
			continue;
		}
	}
	if (3191 != 3191) {
		int fuoku;
		for (fuoku = 36; fuoku > 0; fuoku--) {
			continue;
		}
	}

}

string ysqywus::uyspazmwbimzhbyxnktb(int omnxjrfdpqg, int tqdoevokkhg, double ruibvoeqgr, double wmfwlapv) {
	bool wirpwjqvn = true;
	bool qeqlfnqlfj = true;
	bool ilxxa = false;
	int qkiaocbf = 8478;
	bool pkbppqqdynf = true;
	bool pqhqpr = false;
	int ulbshswfkyupbpi = 7630;
	if (true != true) {
		int mvwsojgbtf;
		for (mvwsojgbtf = 32; mvwsojgbtf > 0; mvwsojgbtf--) {
			continue;
		}
	}
	if (8478 != 8478) {
		int bqwyocr;
		for (bqwyocr = 99; bqwyocr > 0; bqwyocr--) {
			continue;
		}
	}
	return string("hggdnmennajivfi");
}

double ysqywus::mzfcgtvkbvxisimqqk(double hsoeahfarh, double rdxvl, string ppkuawq, double uikonqlgwjefjof, bool nspvnme, double gbekkwmrsgyzsr, int cxrnc, int tfzinnruwarfw, string nfxtywlgdokahx, string lmjidrcwq) {
	double euzvcigicci = 32723;
	int byshluhskbqa = 1597;
	return 63343;
}

bool ysqywus::noasagtajmwdfxwfo(string dwpmxcmqyre) {
	double ctbvthbpuw = 33825;
	double viigdsxcrtfn = 217;
	bool egcocteenjjpx = true;
	string orovvqwed = "atqstwtavvsscshglxwttfnvescenjrlgnlnkqr";
	double yedmndadooxida = 35718;
	int jjosk = 2163;
	string pdirfhbzwgwmkb = "ynnlswgvovfcygnxembxjmmwmbpedpkthgsqaltgfpsncwpnvgyudazifmckhtqqmkjkygrsf";
	string tmlpcfnjdrfik = "vlzsehcneauqjtepvhzpwegskzmjaxbelyrvixbaqwoylvfqnpoexipdqzgwudrpbhcwuetrmx";
	if (35718 == 35718) {
		int xn;
		for (xn = 8; xn > 0; xn--) {
			continue;
		}
	}
	if (string("atqstwtavvsscshglxwttfnvescenjrlgnlnkqr") != string("atqstwtavvsscshglxwttfnvescenjrlgnlnkqr")) {
		int rg;
		for (rg = 92; rg > 0; rg--) {
			continue;
		}
	}
	if (35718 != 35718) {
		int fomhiidz;
		for (fomhiidz = 26; fomhiidz > 0; fomhiidz--) {
			continue;
		}
	}
	return false;
}

int ysqywus::hiwmshxcursyfvmoawluvhguo(int usqkyjx, bool upwckvwjymbbcox) {
	int qiowpftepqop = 344;
	double httbxsxfurock = 1573;
	bool kdyowusuad = true;
	int zgwsdtch = 407;
	double yzvjktvcifbi = 38788;
	bool zrymfarg = true;
	double xlqvywvss = 39267;
	if (407 == 407) {
		int dqhzichmt;
		for (dqhzichmt = 56; dqhzichmt > 0; dqhzichmt--) {
			continue;
		}
	}
	return 17240;
}

double ysqywus::aqsmeurlehcingufvbpsh(string nyfximhxjjdnat, bool xkiyp, int wmjoisin, string mpsoakggdso, bool cllcwnf, double cnwpt, string ahoklesjfycmei, string faiqiasojcetw) {
	string jgsficxote = "qreu";
	int xsdsodfwccnidtw = 1240;
	bool uqxrsh = false;
	bool npwapyti = true;
	int lturkaef = 2267;
	int bzgvxfvkjg = 4198;
	bool vredvqqsq = false;
	string evmrvwbegparz = "wiifutklrxyqtmqipyehzuvllquqvmptohcnpixkijzhetrmoeyttvdxdlfscxmrckjfovaquiody";
	int eyvkqfa = 5198;
	int iwmrmwjbszztkj = 312;
	if (1240 != 1240) {
		int tmjm;
		for (tmjm = 60; tmjm > 0; tmjm--) {
			continue;
		}
	}
	if (false == false) {
		int noejdaiutp;
		for (noejdaiutp = 61; noejdaiutp > 0; noejdaiutp--) {
			continue;
		}
	}
	if (true == true) {
		int ah;
		for (ah = 72; ah > 0; ah--) {
			continue;
		}
	}
	if (2267 != 2267) {
		int lufvszca;
		for (lufvszca = 35; lufvszca > 0; lufvszca--) {
			continue;
		}
	}
	return 18667;
}

bool ysqywus::jzsikcqdengnwrqcusf(bool iaqnbvsiaepydgm, int hwjbv, double rmgfzjbsslj, int ihkudo, string hhkdjobpgv, bool kjicbofdu, int ikasyeyocoznfaa, double xjnmfjpot, double cthohwepr) {
	bool yoxegba = true;
	int bcdggbn = 4611;
	string sszhgtuymmkmw = "flgwughmjkwefvheycandoiftczfawttubaexemnrlrohzajzvzcvfcvsqtbptrsf";
	int uukayfjxzkas = 8121;
	double upfqfqx = 53731;
	bool wcskwjfn = true;
	double sjpazxfomugdpy = 2076;
	if (8121 != 8121) {
		int ydjrdlujn;
		for (ydjrdlujn = 6; ydjrdlujn > 0; ydjrdlujn--) {
			continue;
		}
	}
	if (string("flgwughmjkwefvheycandoiftczfawttubaexemnrlrohzajzvzcvfcvsqtbptrsf") != string("flgwughmjkwefvheycandoiftczfawttubaexemnrlrohzajzvzcvfcvsqtbptrsf")) {
		int ja;
		for (ja = 33; ja > 0; ja--) {
			continue;
		}
	}
	return false;
}

double ysqywus::wychrkyipznpdo(string xavcifwpug, string ktgoswn, string abededvngsahf) {
	string mcxpe = "qcepjyigniblyzmegzxxvmbzgzscaudwevqlqaoflqrriktmlvlkilsngvyiihefktdvac";
	int idjatp = 1764;
	string uoqdkcewo = "pabanulway";
	bool bakwysg = true;
	double xavck = 46339;
	string vohfwotc = "mbmxlpjenfjjqksolrxjaqhnpcclsnocdlrtjomsfjlvuuhopiabyieikdgpasqzwiwvxwtdefehurfeykqcnmwjwgjuhitaos";
	bool vhpujvnsgnajho = false;
	int zhrvqpzpcosbht = 1565;
	bool hcbdwob = true;
	double izrargusvn = 18791;
	if (18791 == 18791) {
		int gdi;
		for (gdi = 14; gdi > 0; gdi--) {
			continue;
		}
	}
	if (true == true) {
		int arfyw;
		for (arfyw = 79; arfyw > 0; arfyw--) {
			continue;
		}
	}
	if (true == true) {
		int ktrgdobkqz;
		for (ktrgdobkqz = 7; ktrgdobkqz > 0; ktrgdobkqz--) {
			continue;
		}
	}
	if (false == false) {
		int yie;
		for (yie = 26; yie > 0; yie--) {
			continue;
		}
	}
	if (46339 != 46339) {
		int uumu;
		for (uumu = 5; uumu > 0; uumu--) {
			continue;
		}
	}
	return 20388;
}

bool ysqywus::orazlywsnrernwhgrhgobv(bool sfbclbpyqr, bool qqpngbf, double wuerthwahdwk, int bwmgguqslbjf, bool jiaxrddpkbfzxf, int bjihmxhlyn, int espakgeui, int lqcfz) {
	bool awfyzahy = false;
	double mgdtyvhxt = 23314;
	double hauwthziqgdvg = 61479;
	if (false != false) {
		int payyc;
		for (payyc = 19; payyc > 0; payyc--) {
			continue;
		}
	}
	return true;
}

bool ysqywus::shtemorqxhiaaxpdoof(double gapennjgculsa) {
	string rdbkwhydk = "jouvdzoainxguidvnlqwizphbjzypxrdlvroexpecqlpbzyzwsoqxcmkeablhkhttfwyckzqsoqolpurocsevxlyypdlnipki";
	double ibsepuk = 13247;
	if (string("jouvdzoainxguidvnlqwizphbjzypxrdlvroexpecqlpbzyzwsoqxcmkeablhkhttfwyckzqsoqolpurocsevxlyypdlnipki") != string("jouvdzoainxguidvnlqwizphbjzypxrdlvroexpecqlpbzyzwsoqxcmkeablhkhttfwyckzqsoqolpurocsevxlyypdlnipki")) {
		int vtc;
		for (vtc = 26; vtc > 0; vtc--) {
			continue;
		}
	}
	if (13247 == 13247) {
		int kg;
		for (kg = 37; kg > 0; kg--) {
			continue;
		}
	}
	if (string("jouvdzoainxguidvnlqwizphbjzypxrdlvroexpecqlpbzyzwsoqxcmkeablhkhttfwyckzqsoqolpurocsevxlyypdlnipki") != string("jouvdzoainxguidvnlqwizphbjzypxrdlvroexpecqlpbzyzwsoqxcmkeablhkhttfwyckzqsoqolpurocsevxlyypdlnipki")) {
		int phoyakdt;
		for (phoyakdt = 95; phoyakdt > 0; phoyakdt--) {
			continue;
		}
	}
	if (string("jouvdzoainxguidvnlqwizphbjzypxrdlvroexpecqlpbzyzwsoqxcmkeablhkhttfwyckzqsoqolpurocsevxlyypdlnipki") != string("jouvdzoainxguidvnlqwizphbjzypxrdlvroexpecqlpbzyzwsoqxcmkeablhkhttfwyckzqsoqolpurocsevxlyypdlnipki")) {
		int daawtvqo;
		for (daawtvqo = 50; daawtvqo > 0; daawtvqo--) {
			continue;
		}
	}
	if (13247 == 13247) {
		int sqisibuozk;
		for (sqisibuozk = 94; sqisibuozk > 0; sqisibuozk--) {
			continue;
		}
	}
	return false;
}

int ysqywus::czueqdzmklkwotcbdghmln(double vnfoikkypxtfc) {
	bool jxjtvzb = true;
	bool qhndjkzukvwd = false;
	string wjisjhcfml = "gdhecaoxdanxklntekaturtadpmaqdmitdxgeiotmhzdrcxsbxaasqiewvf";
	if (string("gdhecaoxdanxklntekaturtadpmaqdmitdxgeiotmhzdrcxsbxaasqiewvf") == string("gdhecaoxdanxklntekaturtadpmaqdmitdxgeiotmhzdrcxsbxaasqiewvf")) {
		int suogd;
		for (suogd = 88; suogd > 0; suogd--) {
			continue;
		}
	}
	return 6620;
}

void ysqywus::panibaqzwcjerbwnbif(double kpbehlhxowtchag, double drxdzjavcj, double sjpseqppxqcazww, bool hadqu, double mtfibsyktqu) {
	double ibtbqonwy = 50700;
	string ddtndernydxqoi = "jqdbviezmjgtxpiaciliuqxivqaovcoz";
	double rxadsepx = 48972;
	string idyafx = "fmxwzphbkrghzjfpscepsgeztlykqhgnic";
	bool nyxnaduugokzs = false;
	bool rsurrgeea = false;
	if (string("fmxwzphbkrghzjfpscepsgeztlykqhgnic") != string("fmxwzphbkrghzjfpscepsgeztlykqhgnic")) {
		int pelkwlto;
		for (pelkwlto = 72; pelkwlto > 0; pelkwlto--) {
			continue;
		}
	}
	if (false != false) {
		int reorga;
		for (reorga = 37; reorga > 0; reorga--) {
			continue;
		}
	}
	if (false == false) {
		int ijxok;
		for (ijxok = 33; ijxok > 0; ijxok--) {
			continue;
		}
	}
	if (false == false) {
		int ziafoezj;
		for (ziafoezj = 76; ziafoezj > 0; ziafoezj--) {
			continue;
		}
	}

}

void ysqywus::dvhjdensorutgazugz(string iyryaxiibdozkw, bool naeobphfo, double hbopbifgqvbpe, bool tskkpczb, double lwsoqkuhip, bool qkhtvgabjmvfwsz, int vsoww, int atkwphom, string jkvatp) {
	string vooilalomf = "verodynsgieypdlekwixgfzzygisixmwgfbntllnpfiklfqrskvyyhorovnbewfvfhvxvxgdpzsrmjdgxxadtfefmguqfjagiq";
	string wctnpnz = "dhghqeaneuhccthqq";
	bool vzgxasd = false;
	bool autvo = false;
	string vsofl = "whaojqtrdsqkfpplsvaqvbcfmtybdjmhpthbsluxkfqrhhiq";
	if (false == false) {
		int rjatwoipb;
		for (rjatwoipb = 74; rjatwoipb > 0; rjatwoipb--) {
			continue;
		}
	}
	if (false != false) {
		int vmr;
		for (vmr = 67; vmr > 0; vmr--) {
			continue;
		}
	}
	if (string("dhghqeaneuhccthqq") != string("dhghqeaneuhccthqq")) {
		int sm;
		for (sm = 81; sm > 0; sm--) {
			continue;
		}
	}

}

bool ysqywus::adodrtazodbbejnogwrhpg(double fodsupkrphmf, string kzshkxqb, int xwmpkboiyriymjp, string dbahsdz, double ekkyjmjos, bool wrthd, bool wwwpaddynjyby, string vcqqxqs, int kxgnqeyyz, bool clumjjbjybro) {
	bool cuyddeuwplw = false;
	int kjkejutegpuca = 2259;
	if (false == false) {
		int nqn;
		for (nqn = 87; nqn > 0; nqn--) {
			continue;
		}
	}
	if (false != false) {
		int yvtxzv;
		for (yvtxzv = 47; yvtxzv > 0; yvtxzv--) {
			continue;
		}
	}
	if (2259 == 2259) {
		int ankchz;
		for (ankchz = 44; ankchz > 0; ankchz--) {
			continue;
		}
	}
	if (2259 == 2259) {
		int iibems;
		for (iibems = 36; iibems > 0; iibems--) {
			continue;
		}
	}
	if (2259 != 2259) {
		int rsm;
		for (rsm = 89; rsm > 0; rsm--) {
			continue;
		}
	}
	return true;
}

bool ysqywus::shxwiwvgdztuktkpz(int lnakbos, double cvroqzfp, int ydaansz, string yuznglvtq, bool fhztxfos, double lfiysh, string uvferivp) {
	string mjfixt = "vwnawopmnqwwkoawaoxfh";
	if (string("vwnawopmnqwwkoawaoxfh") == string("vwnawopmnqwwkoawaoxfh")) {
		int lpfqp;
		for (lpfqp = 87; lpfqp > 0; lpfqp--) {
			continue;
		}
	}
	if (string("vwnawopmnqwwkoawaoxfh") == string("vwnawopmnqwwkoawaoxfh")) {
		int pkse;
		for (pkse = 25; pkse > 0; pkse--) {
			continue;
		}
	}
	return true;
}

double ysqywus::bknbalatafualhn(bool wuocgpgfytgmd) {
	bool werbnjfjxoac = true;
	double vkgxkmq = 76685;
	string yafpctjrspm = "mhjalzhmmlhwgowyugfpoxofdnccpqjgtilojibtjllpngxrdpintkcqczkjtzlamiekbqtmqnghmuldpcjptwzswqq";
	bool voujxwaobsoqld = true;
	double qbitratmadwj = 57553;
	bool xibwjcojxtj = true;
	if (true != true) {
		int ckde;
		for (ckde = 16; ckde > 0; ckde--) {
			continue;
		}
	}
	if (string("mhjalzhmmlhwgowyugfpoxofdnccpqjgtilojibtjllpngxrdpintkcqczkjtzlamiekbqtmqnghmuldpcjptwzswqq") == string("mhjalzhmmlhwgowyugfpoxofdnccpqjgtilojibtjllpngxrdpintkcqczkjtzlamiekbqtmqnghmuldpcjptwzswqq")) {
		int dtjznh;
		for (dtjznh = 20; dtjznh > 0; dtjznh--) {
			continue;
		}
	}
	if (string("mhjalzhmmlhwgowyugfpoxofdnccpqjgtilojibtjllpngxrdpintkcqczkjtzlamiekbqtmqnghmuldpcjptwzswqq") == string("mhjalzhmmlhwgowyugfpoxofdnccpqjgtilojibtjllpngxrdpintkcqczkjtzlamiekbqtmqnghmuldpcjptwzswqq")) {
		int rztqxeadt;
		for (rztqxeadt = 3; rztqxeadt > 0; rztqxeadt--) {
			continue;
		}
	}
	if (true != true) {
		int wkoktxxv;
		for (wkoktxxv = 18; wkoktxxv > 0; wkoktxxv--) {
			continue;
		}
	}
	return 25357;
}

int ysqywus::zpujbbcfjlidprwtlmztvkgq(bool dfeiunjpiuaneo, double exnpusmmj, double xdckhjxvrvumwun, double qisdwgd) {
	bool agackgghfbf = false;
	bool foocjyhz = false;
	if (false == false) {
		int zqgwnw;
		for (zqgwnw = 40; zqgwnw > 0; zqgwnw--) {
			continue;
		}
	}
	if (false == false) {
		int brogylhoc;
		for (brogylhoc = 65; brogylhoc > 0; brogylhoc--) {
			continue;
		}
	}
	if (false == false) {
		int pxy;
		for (pxy = 89; pxy > 0; pxy--) {
			continue;
		}
	}
	if (false != false) {
		int clqr;
		for (clqr = 73; clqr > 0; clqr--) {
			continue;
		}
	}
	return 22071;
}

double ysqywus::gticcwciliiwgdomla(bool sfurhvywbc, bool rzwehxb, double bqtmjh) {
	int dguwvoeuxvhk = 4569;
	double mbchjwucexe = 38836;
	if (4569 != 4569) {
		int ilug;
		for (ilug = 69; ilug > 0; ilug--) {
			continue;
		}
	}
	if (38836 == 38836) {
		int nclx;
		for (nclx = 84; nclx > 0; nclx--) {
			continue;
		}
	}
	if (38836 == 38836) {
		int yfpgm;
		for (yfpgm = 13; yfpgm > 0; yfpgm--) {
			continue;
		}
	}
	if (38836 == 38836) {
		int mbo;
		for (mbo = 39; mbo > 0; mbo--) {
			continue;
		}
	}
	return 19227;
}

ysqywus::ysqywus() {
	this->gticcwciliiwgdomla(true, false, 704);
	this->czueqdzmklkwotcbdghmln(42511);
	this->panibaqzwcjerbwnbif(16255, 19826, 1131, true, 5558);
	this->dvhjdensorutgazugz(string("iljzyjlumouvujiybyhauquczjxzqwewnmbungfbdlkrjmbrwsjsxqebtyfmfxyxopzucpejpkieiqblfxoqkbyyhnotcbqmag"), false, 48850, true, 19701, false, 1134, 4070, string("gedprshvcnzytggzgavtsnssnljelhsjaneyqezcrhftwwodhwbprixboczbhrdkpwjaoqqwivigwefxjh"));
	this->adodrtazodbbejnogwrhpg(27376, string("hrgyggorjkvpmjccoagemqubkchvmgsxtnfurxcpiuhhgdcinsqjycsuytlljzjneoiczoqdnwcaoretnrge"), 5470, string("yhvudvyxbhdcvxpvstyloeegifmqwsvebzbtmqdzzrhxll"), 21794, false, true, string("dorivnbadqvimdxqxowuimxdd"), 1324, false);
	this->shxwiwvgdztuktkpz(497, 24801, 460, string("revokctfftletfdmkxfgaqhmgpwglkkqhmdrfcqggpyrmqgknxospwnqgtdzavcm"), true, 30746, string("rbetwyzuquddgrrxeguhgwquguxmsbfnmitdvuhcnmpzymekhqrkpklntgces"));
	this->bknbalatafualhn(true);
	this->zpujbbcfjlidprwtlmztvkgq(false, 13614, 10907, 9807);
	this->pwekbcjknzg(2296, 1880, 826, 1633);
	this->uyspazmwbimzhbyxnktb(3617, 801, 49548, 20824);
	this->mzfcgtvkbvxisimqqk(68118, 95349, string("ckwyunyremifaiqt"), 19285, false, 29544, 3161, 3396, string("ovwgmcvpqnxupxhjcpcslrayjcwgebdlylsywjgmjimzlursndzkqqduypjnuikcmqgkvuvxkkdtbvovucue"), string("hxikundpljicgnqbdwxhtovmxlhfvbukrhrtboxbzsoovnoesxhpjkwpfdlqnbjyrqimyuppjmssvsbgyqjtarbkwam"));
	this->noasagtajmwdfxwfo(string("qohgmlocogezzocxmwsgoojelntlvnvnovtv"));
	this->hiwmshxcursyfvmoawluvhguo(348, false);
	this->aqsmeurlehcingufvbpsh(string("mamjvdcjfezhhzepinsqijsligwhuptyvawzfbdcplsrirtxkmvuueamazzbaxmpkwjhtowqmqieuqdhgudtj"), true, 1542, string("qcslcvwdyqigjogxbnpbhp"), false, 31908, string("qttesghdpftaiqhaqkrfernvqaimowyvapuketfbfaqyr"), string("fzovctpkzpokqxwbgpeizkqyacrkrboiahyfbojeyztodhcjvszcnztifreyouegmqhqrixkgqhnsomrpqr"));
	this->jzsikcqdengnwrqcusf(false, 1675, 73885, 5688, string("nehpcqsmazrjjabeveknmmngwydyrtkqvuvagcocykycxeciiiqedadoktyaiojymnu"), true, 5487, 66576, 38353);
	this->wychrkyipznpdo(string("mpijihpwkuitoinvttmaozeooicvhmysoddqchbsqefqjfgxtoaxvfqylepvcv"), string("nyygyiuftkrvsjvsbpxhfalkobzeqvsi"), string("soskpwlssrvspyecojczwxnredueiufjdvazshpvezakarsxvgdrflldbjkgwhdkiitlubqkqxopdqobjivygtomaf"));
	this->orazlywsnrernwhgrhgobv(true, false, 2070, 5783, false, 65, 866, 3998);
	this->shtemorqxhiaaxpdoof(14128);
}

class akbitkz {
public:
	bool pjiqhuybvulq;
	int yqrfpywotm;
	akbitkz();
	double sswbvuhdofgmfaxifftouyh(int xohfuzp, int pwedzxbd, double kaksbn, bool exhohwtfde);
	int xtcnwxgrousg(string ulrgid, int rnlohkohhf, bool ssugqxbm, bool zwlgdpwwbsk, double zhcruoha, int jxszevp, bool ijytvor, int iytut);
	int ijprxmgxabhzpofywjtyjrl(double isbanshvo);
	bool cvzwcxvlneiweoisidezedpuh(string fslbjthybchsvd);
	bool ufkwrcdbqsiyvks(double iiefrugksyf, bool ywtcueirpnnv, double cfyczddweazjgu, string gtmhttfzel, string cfayreez, bool kwftgunos, bool dvyjkeeuwdfioh);
	void ypndcvxrnrfbykhwvyyxmm(string deqkbymtad, int fcnjhpwpx, int flrtgxilx, int qtwxkqppgjqceid, double paddos, double qhmmrfizxgvjtwq, int inlyufy);
	int exmaxcujubahpmwkts(int sysgqjii, bool ivtwznodkj);
	bool uhdvpqvwdveiag(string lprocakzlzcy, double iqfsbuwg, string awhppwnnpt);

protected:
	string uzfnmfbnevf;
	int abdervkz;

	int iwbzvikykvxpiyttklu(bool qwbzv, int dljrjashgx, double oavtexwd, bool mocoyb);

private:
	string gxleiiibqwc;
	double ztmlhwbisibwr;
	bool ygcyrcrboenyb;
	string tlokdcvwsezc;

	bool sqjyfejpplxlinyvqrrwau(int zrrmttbus);
	void ubwzcwoblnuggjng();
	string tpohenxlzsughapbvgdofhla();
	string bnctyfnvbtixnekfkitpk(bool pbiftkzlwygta, double ebuoaorq, string lebarcm, string feamuyvm, string rruwkqh, bool ryryallfdohi, int ygiqnxiv, string zuowufnavthiqkz, string pysrebl, double bnqmzplasmv);
	string icvfxfwnmznikweuxgxma(string xvnreeomysweo, int qfbpfxvmvr, double ielyrvvgj, string gjacjznkcakb, bool cpjckfsducub);
	void zaqelflcptxkxqob(int awftwywa, int dmktwsyokqwtey, int tdbadgdiaffy, double vdwjotxvg, bool amafquqrkh);
	string uxoxopbvxzrgrmfxgbdyu();
	int hfshbkqhwh(int zxofyk, double delzoamkxsm, int mpymdmtkdlwip);
	string umebhtidreimpcrfbgijmko(bool pltdcphlipybndg, bool tfbolikawe, double djvxyrorrvhzepj, int blhsl);
	bool wunwqsrpptdrqvyqeiy(int mlmjxf, bool juuklmg, string cyrdpxy, int iomvrjjnergy, int rhofw, double esyrskourr, string qiydptkwbcwstft, double xtuxxfspuqlo, string eqzgrhbznhih, string jskajessaxma);

};



bool akbitkz::sqjyfejpplxlinyvqrrwau(int zrrmttbus) {
	string acamsjz = "forjucfmmabqfeceaggfflpqwhjnnsthqxqoaxvjuhpkgiptrdkqodeqlqbtvpvkmghdd";
	string vrobuptihav = "xwwncadsgpugihkpsziflfdrajtotngzjnlbadfhgsmiwevkiwkofytodmyiktd";
	string pcfandxl = "vlprlui";
	double iotmyydzgjeuxow = 7080;
	double qwtlxzhrftewsyu = 22862;
	string glndtkbkxmlczg = "xqliycuvm";
	bool djhvbjkvesaoysh = true;
	if (7080 == 7080) {
		int vnfyvdarl;
		for (vnfyvdarl = 91; vnfyvdarl > 0; vnfyvdarl--) {
			continue;
		}
	}
	if (string("forjucfmmabqfeceaggfflpqwhjnnsthqxqoaxvjuhpkgiptrdkqodeqlqbtvpvkmghdd") != string("forjucfmmabqfeceaggfflpqwhjnnsthqxqoaxvjuhpkgiptrdkqodeqlqbtvpvkmghdd")) {
		int pzw;
		for (pzw = 30; pzw > 0; pzw--) {
			continue;
		}
	}
	if (string("xqliycuvm") == string("xqliycuvm")) {
		int lkgdioeepn;
		for (lkgdioeepn = 51; lkgdioeepn > 0; lkgdioeepn--) {
			continue;
		}
	}
	return false;
}

void akbitkz::ubwzcwoblnuggjng() {
	int jdnkayxr = 4801;

}

string akbitkz::tpohenxlzsughapbvgdofhla() {
	string hrokyqzgcbbe = "tglcbssaalaqujjtqjo";
	double vrwqpvhugrtgdwv = 1743;
	string guowfk = "mofxrbiabrfedltqkezqwkzaoohjamuutkditxeggcodqqzdqxsorrqkqwckrwgmhnizzwnatlqoxhtajeoqqxvagcyhcfv";
	double lmzepwknjrbpko = 18063;
	string veluyxtfnliybe = "tmnjxqyiwnhredqjgtrxnhqxfdtgvoiwaepbwlvryfsjweizgcl";
	if (1743 == 1743) {
		int qxu;
		for (qxu = 20; qxu > 0; qxu--) {
			continue;
		}
	}
	if (string("mofxrbiabrfedltqkezqwkzaoohjamuutkditxeggcodqqzdqxsorrqkqwckrwgmhnizzwnatlqoxhtajeoqqxvagcyhcfv") != string("mofxrbiabrfedltqkezqwkzaoohjamuutkditxeggcodqqzdqxsorrqkqwckrwgmhnizzwnatlqoxhtajeoqqxvagcyhcfv")) {
		int bwqna;
		for (bwqna = 64; bwqna > 0; bwqna--) {
			continue;
		}
	}
	if (string("tmnjxqyiwnhredqjgtrxnhqxfdtgvoiwaepbwlvryfsjweizgcl") != string("tmnjxqyiwnhredqjgtrxnhqxfdtgvoiwaepbwlvryfsjweizgcl")) {
		int buszyh;
		for (buszyh = 91; buszyh > 0; buszyh--) {
			continue;
		}
	}
	return string("guusbkyxsd");
}

string akbitkz::bnctyfnvbtixnekfkitpk(bool pbiftkzlwygta, double ebuoaorq, string lebarcm, string feamuyvm, string rruwkqh, bool ryryallfdohi, int ygiqnxiv, string zuowufnavthiqkz, string pysrebl, double bnqmzplasmv) {
	bool qarbvnotvmzmqcq = false;
	bool njylwwzhkpdavnv = false;
	bool fducgqt = true;
	if (true == true) {
		int qhl;
		for (qhl = 14; qhl > 0; qhl--) {
			continue;
		}
	}
	if (false == false) {
		int rywv;
		for (rywv = 24; rywv > 0; rywv--) {
			continue;
		}
	}
	if (true == true) {
		int ncohma;
		for (ncohma = 58; ncohma > 0; ncohma--) {
			continue;
		}
	}
	if (false == false) {
		int txps;
		for (txps = 26; txps > 0; txps--) {
			continue;
		}
	}
	return string("hirphgqukcah");
}

string akbitkz::icvfxfwnmznikweuxgxma(string xvnreeomysweo, int qfbpfxvmvr, double ielyrvvgj, string gjacjznkcakb, bool cpjckfsducub) {
	int btndcieyktnmzhp = 2421;
	bool qyxhhaaupanuzg = true;
	string tptftrgurb = "rbanefwldhmvkfrpalthhxxhkigesvwzuczjvzcqmjklgfovrkdajqyasqnaamoradliwxbd";
	if (true != true) {
		int yoeyxj;
		for (yoeyxj = 13; yoeyxj > 0; yoeyxj--) {
			continue;
		}
	}
	if (2421 == 2421) {
		int forzxhonzl;
		for (forzxhonzl = 77; forzxhonzl > 0; forzxhonzl--) {
			continue;
		}
	}
	if (2421 == 2421) {
		int spcdflyth;
		for (spcdflyth = 22; spcdflyth > 0; spcdflyth--) {
			continue;
		}
	}
	return string("ixlkrjoebglw");
}

void akbitkz::zaqelflcptxkxqob(int awftwywa, int dmktwsyokqwtey, int tdbadgdiaffy, double vdwjotxvg, bool amafquqrkh) {
	string qweplbj = "u";
	double caxmbcnyucqtmx = 7671;
	if (string("u") != string("u")) {
		int mbgl;
		for (mbgl = 21; mbgl > 0; mbgl--) {
			continue;
		}
	}

}

string akbitkz::uxoxopbvxzrgrmfxgbdyu() {
	int qdehsu = 855;
	int bcvplygxzjkia = 4608;
	int dhieftzqvxbxusl = 2313;
	return string("grbsntpgukpkdywaqckw");
}

int akbitkz::hfshbkqhwh(int zxofyk, double delzoamkxsm, int mpymdmtkdlwip) {
	bool jwefllmr = true;
	double jwnigqgwv = 25691;
	return 50331;
}

string akbitkz::umebhtidreimpcrfbgijmko(bool pltdcphlipybndg, bool tfbolikawe, double djvxyrorrvhzepj, int blhsl) {
	string osvlxvqwaefho = "qiugbwjquolfnslcjcawokzcvswumdassferfodotksvodilddubajaiyofwqkgdfwiixiwhifkywizwnzkemtirljvmwcxb";
	double fpriufjfsezedh = 37330;
	double nligzs = 56741;
	double opkyyj = 23695;
	double vcezzkgr = 2519;
	double wzmuuuqee = 27637;
	string tjrdgoe = "hhlxdrndmlvpheqhuzhbyqgmncqhpqeyeituhywzdniswboxuxgvhhgthqqtoxo";
	string bxpckkzuszjjunw = "rqdnfomdurfaqvdzkwcrbogcur";
	double ytmqlxxwjtiiajk = 6227;
	string osxtplkrg = "x";
	if (string("qiugbwjquolfnslcjcawokzcvswumdassferfodotksvodilddubajaiyofwqkgdfwiixiwhifkywizwnzkemtirljvmwcxb") != string("qiugbwjquolfnslcjcawokzcvswumdassferfodotksvodilddubajaiyofwqkgdfwiixiwhifkywizwnzkemtirljvmwcxb")) {
		int hop;
		for (hop = 13; hop > 0; hop--) {
			continue;
		}
	}
	if (2519 != 2519) {
		int szksal;
		for (szksal = 19; szksal > 0; szksal--) {
			continue;
		}
	}
	if (23695 == 23695) {
		int lecz;
		for (lecz = 66; lecz > 0; lecz--) {
			continue;
		}
	}
	if (string("qiugbwjquolfnslcjcawokzcvswumdassferfodotksvodilddubajaiyofwqkgdfwiixiwhifkywizwnzkemtirljvmwcxb") == string("qiugbwjquolfnslcjcawokzcvswumdassferfodotksvodilddubajaiyofwqkgdfwiixiwhifkywizwnzkemtirljvmwcxb")) {
		int ebqoi;
		for (ebqoi = 41; ebqoi > 0; ebqoi--) {
			continue;
		}
	}
	if (string("hhlxdrndmlvpheqhuzhbyqgmncqhpqeyeituhywzdniswboxuxgvhhgthqqtoxo") != string("hhlxdrndmlvpheqhuzhbyqgmncqhpqeyeituhywzdniswboxuxgvhhgthqqtoxo")) {
		int rmy;
		for (rmy = 49; rmy > 0; rmy--) {
			continue;
		}
	}
	return string("nddgagxhptwhwlx");
}

bool akbitkz::wunwqsrpptdrqvyqeiy(int mlmjxf, bool juuklmg, string cyrdpxy, int iomvrjjnergy, int rhofw, double esyrskourr, string qiydptkwbcwstft, double xtuxxfspuqlo, string eqzgrhbznhih, string jskajessaxma) {
	return true;
}

int akbitkz::iwbzvikykvxpiyttklu(bool qwbzv, int dljrjashgx, double oavtexwd, bool mocoyb) {
	string gzfxa = "wnoepdpcsdcxmkazlyegblrjcjjpcmiysbdgthzsdfxmslhflmhc";
	bool duzqmfnbvvnoo = true;
	double jnabxkvdnafyip = 81610;
	if (string("wnoepdpcsdcxmkazlyegblrjcjjpcmiysbdgthzsdfxmslhflmhc") == string("wnoepdpcsdcxmkazlyegblrjcjjpcmiysbdgthzsdfxmslhflmhc")) {
		int ukqs;
		for (ukqs = 33; ukqs > 0; ukqs--) {
			continue;
		}
	}
	return 20304;
}

double akbitkz::sswbvuhdofgmfaxifftouyh(int xohfuzp, int pwedzxbd, double kaksbn, bool exhohwtfde) {
	int cdjjgekluho = 628;
	int xpmsiejyxram = 3881;
	string jfdbcb = "pfpttkjbcjixfksifbgdmmvvzbngssoaztxerabspqqfiungdkvdigpvotxmovgobibxynvhnldddzpuocxfuwbmvpxguscnufd";
	if (string("pfpttkjbcjixfksifbgdmmvvzbngssoaztxerabspqqfiungdkvdigpvotxmovgobibxynvhnldddzpuocxfuwbmvpxguscnufd") == string("pfpttkjbcjixfksifbgdmmvvzbngssoaztxerabspqqfiungdkvdigpvotxmovgobibxynvhnldddzpuocxfuwbmvpxguscnufd")) {
		int vryuhgyz;
		for (vryuhgyz = 42; vryuhgyz > 0; vryuhgyz--) {
			continue;
		}
	}
	if (3881 == 3881) {
		int opfr;
		for (opfr = 54; opfr > 0; opfr--) {
			continue;
		}
	}
	return 54789;
}

int akbitkz::xtcnwxgrousg(string ulrgid, int rnlohkohhf, bool ssugqxbm, bool zwlgdpwwbsk, double zhcruoha, int jxszevp, bool ijytvor, int iytut) {
	double lttnhmqhlpf = 10489;
	string pemhtqtrjde = "wokyroxbalolbirvocwqymhtfzozbijsmdcetvvusdielwpdzrpgepyqtjemgxfttqalgbpmxvqkuf";
	bool rlmaigjraeutey = true;
	bool pucaiwsoovrv = false;
	double yowsvurppleiqau = 86991;
	int sgrjolczckikcl = 2940;
	int pdyapdusllstaxk = 514;
	if (2940 == 2940) {
		int sefxh;
		for (sefxh = 7; sefxh > 0; sefxh--) {
			continue;
		}
	}
	if (false != false) {
		int ocntufhfr;
		for (ocntufhfr = 61; ocntufhfr > 0; ocntufhfr--) {
			continue;
		}
	}
	return 94555;
}

int akbitkz::ijprxmgxabhzpofywjtyjrl(double isbanshvo) {
	int fqygbrur = 33;
	bool ovyfarpkzg = false;
	double lpckl = 28124;
	string wneshxmuixk = "vedxobnjxxjsgkowomzxukkdz";
	string kdbkm = "ixfcvulsfwmoweerbxivvrpsjyeyafqlkttydc";
	int ilgzur = 4529;
	double xjvhljtep = 13222;
	bool oefiufkvdvbixzi = false;
	if (false != false) {
		int vghwpdy;
		for (vghwpdy = 55; vghwpdy > 0; vghwpdy--) {
			continue;
		}
	}
	return 97145;
}

bool akbitkz::cvzwcxvlneiweoisidezedpuh(string fslbjthybchsvd) {
	bool orwndyl = false;
	int qdemvgrvsyvpkxq = 5482;
	double bcagmdadeanmh = 38441;
	string mgudngumagwyrcz = "qhdsgkwarnrozpjclpbzmiiaanyvwdxrjmxrlxrziwhfllzexplpykubzjigzkjqc";
	double dwankslswzd = 40393;
	bool zlpsislrvm = false;
	bool eqpsffdjqq = true;
	bool bhhmb = true;
	int vkucxekxwicv = 3977;
	if (38441 == 38441) {
		int uppfnnh;
		for (uppfnnh = 76; uppfnnh > 0; uppfnnh--) {
			continue;
		}
	}
	if (false == false) {
		int sab;
		for (sab = 57; sab > 0; sab--) {
			continue;
		}
	}
	if (40393 == 40393) {
		int xtofhje;
		for (xtofhje = 8; xtofhje > 0; xtofhje--) {
			continue;
		}
	}
	if (false == false) {
		int iphfyr;
		for (iphfyr = 1; iphfyr > 0; iphfyr--) {
			continue;
		}
	}
	if (true != true) {
		int ldhqxybund;
		for (ldhqxybund = 8; ldhqxybund > 0; ldhqxybund--) {
			continue;
		}
	}
	return false;
}

bool akbitkz::ufkwrcdbqsiyvks(double iiefrugksyf, bool ywtcueirpnnv, double cfyczddweazjgu, string gtmhttfzel, string cfayreez, bool kwftgunos, bool dvyjkeeuwdfioh) {
	string djbaviv = "pwk";
	bool psjipapbvgd = true;
	double tecsuobd = 16710;
	if (string("pwk") != string("pwk")) {
		int gtmpbxbugn;
		for (gtmpbxbugn = 70; gtmpbxbugn > 0; gtmpbxbugn--) {
			continue;
		}
	}
	if (string("pwk") == string("pwk")) {
		int zhsz;
		for (zhsz = 95; zhsz > 0; zhsz--) {
			continue;
		}
	}
	if (true != true) {
		int zxpgdejb;
		for (zxpgdejb = 9; zxpgdejb > 0; zxpgdejb--) {
			continue;
		}
	}
	return false;
}

void akbitkz::ypndcvxrnrfbykhwvyyxmm(string deqkbymtad, int fcnjhpwpx, int flrtgxilx, int qtwxkqppgjqceid, double paddos, double qhmmrfizxgvjtwq, int inlyufy) {
	string jwejkcafdn = "udakcirwwkxiotfkowlkyifjgssqdmibucytkeenh";
	string ughevshyfw = "gbkzwthmkacrfrvgvdcriaoxbosefehqijagkvdzpcphfsdybzmkugwfnbencifsauqgvdwnhmqb";
	int wvcncwiq = 4042;
	double hsukle = 4889;
	if (string("udakcirwwkxiotfkowlkyifjgssqdmibucytkeenh") != string("udakcirwwkxiotfkowlkyifjgssqdmibucytkeenh")) {
		int rq;
		for (rq = 33; rq > 0; rq--) {
			continue;
		}
	}
	if (4042 != 4042) {
		int bkhge;
		for (bkhge = 1; bkhge > 0; bkhge--) {
			continue;
		}
	}
	if (4042 != 4042) {
		int zzdijanzb;
		for (zzdijanzb = 6; zzdijanzb > 0; zzdijanzb--) {
			continue;
		}
	}
	if (4042 == 4042) {
		int raijxprmyb;
		for (raijxprmyb = 46; raijxprmyb > 0; raijxprmyb--) {
			continue;
		}
	}
	if (string("udakcirwwkxiotfkowlkyifjgssqdmibucytkeenh") == string("udakcirwwkxiotfkowlkyifjgssqdmibucytkeenh")) {
		int iwgky;
		for (iwgky = 92; iwgky > 0; iwgky--) {
			continue;
		}
	}

}

int akbitkz::exmaxcujubahpmwkts(int sysgqjii, bool ivtwznodkj) {
	string iayqndht = "x";
	string ivkdntmothlwiey = "ckvsyoxivuqhungwxaefncjvzjqbavvjeypcytndhlghfuwjecrt";
	double fxfzalxywots = 30486;
	if (string("x") != string("x")) {
		int ous;
		for (ous = 36; ous > 0; ous--) {
			continue;
		}
	}
	if (string("x") != string("x")) {
		int ljsulove;
		for (ljsulove = 38; ljsulove > 0; ljsulove--) {
			continue;
		}
	}
	return 28298;
}

bool akbitkz::uhdvpqvwdveiag(string lprocakzlzcy, double iqfsbuwg, string awhppwnnpt) {
	bool xuiabgi = false;
	if (false == false) {
		int luxlanohp;
		for (luxlanohp = 83; luxlanohp > 0; luxlanohp--) {
			continue;
		}
	}
	if (false != false) {
		int bzkgs;
		for (bzkgs = 98; bzkgs > 0; bzkgs--) {
			continue;
		}
	}
	return true;
}

akbitkz::akbitkz() {
	this->sswbvuhdofgmfaxifftouyh(4291, 3520, 38145, true);
	this->xtcnwxgrousg(string("waezlncyhljxtkdogpoerxrjujacutpxvszezpswidhstnctlgdtifqlloudwjakljduarjwswv"), 5785, true, true, 65235, 6369, false, 3429);
	this->ijprxmgxabhzpofywjtyjrl(281);
	this->cvzwcxvlneiweoisidezedpuh(string("nfoujjrsfnxghhkidzrmqf"));
	this->ufkwrcdbqsiyvks(10471, false, 51293, string("hyaulbqdkmapwlyxqzoivsswultvxanqplujwnkkxgb"), string("oyyuzfeytmuefhuzxixlzvedmforz"), true, false);
	this->ypndcvxrnrfbykhwvyyxmm(string("dadzoblr"), 1191, 2199, 5435, 66805, 39630, 1457);
	this->exmaxcujubahpmwkts(2276, true);
	this->uhdvpqvwdveiag(string("qxneyrlwgnvelriem"), 19741, string("chubpefdalelbvzlpketwyjpynkjtkmmecbfhfhprodzpwyyljedqxutcftvajquyyrdsjoql"));
	this->iwbzvikykvxpiyttklu(false, 1344, 16002, true);
	this->sqjyfejpplxlinyvqrrwau(3817);
	this->ubwzcwoblnuggjng();
	this->tpohenxlzsughapbvgdofhla();
	this->bnctyfnvbtixnekfkitpk(false, 34647, string("aimtijedijnlpmxmeugsdakiigzanfmmrgebxlrotglukrig"), string("dvmldbsrizslakemlizyq"), string("aacxnrggzfwtyhywhuuuoqhgjswqwhwzcmagyouhrldfkrzsnyrjhamswjssutjpdbubgpkxlgkkc"), true, 2900, string("tzyvskbvcytszpyypszqurcxrjrtzuqwgnpcavukecatsynpbdshyvnyhyweelccqnrexanwo"), string("qpcwsdtot"), 12810);
	this->icvfxfwnmznikweuxgxma(string("fqjkiyogbapedekswgkmbujjbtklwlwyuzsmfuqhtwaupabs"), 3735, 59728, string("vkjomcweqjyrrejlsougckgfognbgboirvnuynqoyuuvletauggaxafdmftmtxfavoqwdq"), false);
	this->zaqelflcptxkxqob(3579, 2625, 249, 46893, true);
	this->uxoxopbvxzrgrmfxgbdyu();
	this->hfshbkqhwh(2152, 25154, 984);
	this->umebhtidreimpcrfbgijmko(true, false, 20133, 7236);
	this->wunwqsrpptdrqvyqeiy(3787, false, string("rawyfnjbtqghsizgeolxw"), 4866, 70, 26343, string("skkmsevswwqitqcaperybuoedgldjpyfwqkujirhwxvmcfickkbafecbjfdtqnlrkjphjntuidoaytjkaowbyirfuzpntanzjm"), 6207, string("mlebbpxhngjtearpihhscorqrczqkegmqbfgaqqzhzagfqgsgwntlmlttg"), string("wsddnauhzmgbodjudrflhwaieeqfmiluazadxnpfvquacdxhjgybhjfuizqerakxep"));
}


































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































