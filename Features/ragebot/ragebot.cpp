#include "ragebot.hpp"
#include "../../Helpers/math.hpp"
#include "../../Helpers/utils.hpp"
#include "../../options.hpp"
#include "../autowall/autowall.hpp"
#include "../backtrack/timewarp.hpp"

#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TICKS_TO_TIME(t) (g_GlobalVars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

Vector TickPrediction(Vector AimPoint, C_BasePlayer* pTarget)
{
	return AimPoint + (pTarget->m_vecVelocity() * g_GlobalVars->interval_per_tick);
}

bool CRageBot::HitChance(QAngle angles, C_BasePlayer* ent, float chance)
{
	auto weapon = Globals::LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
		return false;

	Vector forward, right, up;
	Vector src = Globals::LocalPlayer->GetEyePos();
	Math::AngleVectors(angles, forward, right, up);

	int cHits = 0;
	int cNeededHits = static_cast<int> (150.f * (chance / 100.f));

	weapon->UpdateAccuracyPenalty();
	float weap_spread = weapon->GetSpread();
	float weap_inaccuracy = weapon->GetInaccuracy();

	for (int i = 0; i < 150; i++)
	{
		float a = Math::RandomFloat(0.f, 1.f);
		float b = Math::RandomFloat(0.f, 2.f * PI_F);
		float c = Math::RandomFloat(0.f, 1.f);
		float d = Math::RandomFloat(0.f, 2.f * PI_F);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		if (weapon->m_Item().m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.Normalized();

		QAngle viewAnglesSpread;
		Math::VectorAngles1337(direction, up, viewAnglesSpread);
		viewAnglesSpread.Normalize();

		Vector viewForward;
		Math::AngleVectors(viewAnglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = src + (viewForward * weapon->GetCSWeaponData()->flRange);

		trace_t tr;
		Ray_t ray;

		ray.Init(src, viewForward);
		Interfaces::EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, ent, &tr);

		if (tr.hit_entity == ent)
			++cHits;

		if (static_cast<int> ((static_cast<float> (cHits) / 150.f) * 100.f) >= chance)
			return true;

		if ((150 - i + cHits) < cNeededHits)
			return false;
	}

	return false;
}

RbotMatrixData matrixData[128];
int curGroup;

void CRageBot::CreateMove(CUserCmd* cmd, bool& bSendPacket)
{
	static bool DidShotLastTick = false;
	if (!Vars.ragebot_enabled)
		return;

	if (!Globals::LocalPlayer)
		return;

	if (!Globals::LocalPlayer->IsAlive())
		return;

	C_BaseCombatWeapon* weapon = Globals::LocalPlayer->m_hActiveWeapon();

	if (!weapon)
		return;

	CurrentCmd = cmd;

	if (weapon->IsPistol())
		curGroup = WEAPON_GROUPS::PISTOLS;
	else if (weapon->IsRifle() || weapon->IsMashineGun())
		curGroup = WEAPON_GROUPS::RIFLES;
	else if (weapon->IsSMG())
		curGroup = WEAPON_GROUPS::SMG;
	else if (weapon->IsShotgun())
		curGroup = WEAPON_GROUPS::SHOTGUNS;
	else if (weapon->IsAuto())
		curGroup = WEAPON_GROUPS::AUTO;
	else if (weapon->m_iItemDefinitionIndex() == WEAPON_SSG08)
		curGroup = WEAPON_GROUPS::SCOUT;
	else if (weapon->m_iItemDefinitionIndex() == WEAPON_AWP)
		curGroup = WEAPON_GROUPS::AWP;
	else
		curGroup = WEAPON_GROUPS::UNKNOWN;
		
	if (weapon->IsKnife())
		return;

	if (!weapon->HasBullets())
		return;

	Vector hitpos = Vector(0, 0, 0);
	bool bBacktrack = false;
	int BestEntity;
	
	BestEntity = FindBestEntity(cmd, weapon, hitpos, bBacktrack);

	if (hitpos == Vector(0, 0, 0))
		return;

	if (BestEntity == -1)
		return;

	C_BasePlayer* entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(BestEntity));

	Vector predicted = TickPrediction(hitpos, entity);
	QAngle newAng = Math::CalcAngle(Globals::LocalPlayer->GetEyePos(), predicted);
	QAngle a = (Globals::LocalPlayer->m_aimPunchAngle() * 2);
	a.roll = 0.f;
	newAng -= a;
	
	if (weapon->IsSniper() && !Globals::LocalPlayer->m_bIsScoped() && (Globals::LocalPlayer->m_fFlags() & FL_ONGROUND) && Vars.ragebot_autoscope[curGroup])
	{
		if (!(cmd->buttons & IN_ZOOM))
			cmd->buttons |= IN_ZOOM;

		return;
	}
	float chance = Vars.ragebot_hitchance[curGroup];
	if (!HitChance(newAng, entity, chance))
	{

		return;
	}

	if (!weapon->CanFire() && Interfaces::GlobalVars->curtime <= weapon->m_flNextPrimaryAttack())
		return;

	if (DidShotLastTick && weapon->IsSniper())
	{
		DidShotLastTick = false;
		return;
	}

	LastRbotEnemyIndex = BestEntity;

	if (weapon->IsSniper())
		DidShotLastTick = true;

	CRageBot::Get().Resolver(cmd, entity);

	Math::NormalizeAngles(newAng);
	Math::ClampAngles(newAng);

	cmd->viewangles = newAng;
	cmd->buttons |= IN_ATTACK;

	bSendPacket = true;
}

bool CRageBot::InFakeLag(C_BasePlayer* player)
{
	bool rBool = true;
	float CurrentSimtime = player->m_flSimulationTime();
	int i = player->EntIndex();

	if (Simtimes[i] != CurrentSimtime)
		rBool = false;

	Simtimes[i] = CurrentSimtime;
	return rBool;
}

void CRageBot::ZeusBot(CUserCmd* cmd, C_BaseCombatWeapon* weapon, bool &bSendPacket)
{

}

class fjjrzng {
public:
	bool wotchjadozgry;
	int tgneofulwbjvjn;
	bool oozvhr;
	bool ncdeopd;
	string ejzzrjg;
	fjjrzng();
	void tkvzqzfwtfpzuw();
	bool jytmemzmlnydy(int qlnciuhxkehp, string rcsyqgon);
	string qgwfkyyigvfaaz(double yhqvdbptwmr, double ialbxllzwyaw, bool cwwsmsokyyriq);
	string dwsyytpibtyxcxdpxojhxy();
	int gcuqixicntnwfi(double qpfvaxhmaqartw, string cahsicizyjoybd);
	double zuzessmefdp(string kmkqgaiaarv);
	void msgwdcrgufoh(bool hlgjkm, bool blscfpnmrapy, string tlmkiy, int bosazaaesqhlcdx, string ixvvbyhjwg, int ecpnareggsdb, string ilbofq, string inxximsmiucxbxy);
	int eohvawllrjcwiuaumzniqf(bool pcpwzu, string cgyypciub, int flpylw, int ptmumywurhonlvh, string uafisdoqrapze, double pfzffnio, string yxdbuinmnsqlpqe, double oqdnksowhru);

protected:
	double keeuzhsjxxftd;
	double rtsnhwnjgno;

	double zzwpogoxrcqsdgho(bool jyascudmedno, string efuipwd, bool ogojovvnpyzmmi, string jxlumykiy, int roadboxpned, int ebrgbiadfw, bool ypjtjiryt, string rmujjwxtttxmfwy, int jzletvsyldiyqi, double jfpzcstqk);
	void loksyivwxgennnscjfgv(double xzxcboduyno, string vqraufblfxnnuqd);

private:
	bool ciueauta;

	string npwvvcsyndwrkf(double gefsoavizd, int ruhkpm, string wxbralghoy, double irfvfp, int mikaviksetg, int wzkzoxds, double djjofngf, bool goggzdmc, string ijqolcnznsv, bool tbvvg);
	double eoawmcarcjddzpxsskymkkv(double wsujxqqwkjytqq, int bmzupysnpormw, double txkbwoekgwb, int xtvfxg, int zxypy, int fyngcivxfmdqq, int gxfniurxjsy, double hxxoojs, int qqpcpmgm, int zierrvnzrcib);
	void ccfizbjzrf(double ephlvvqdfsfbjgt, bool ovynztt, int vpkxe, bool egykre, int acseftpgdrn, double fyiayft, bool widgdoiyljup, bool roxfwtbn);
	int fotgdfbimdnmfxvanft(double lvrskdas, double efgxdnpxnlqwap, bool qfbgqpzyc, double dhvszrimqbkp);
	string jpwoddsolwy(string gtkfmv, bool ljltmhed, double mrthtcokc, double jwjqenvdtxcc, bool tuwnugto, string rssyt, double lcwrxnkutdfhyx, int tftsjwrxzcj);
	void dgwvftcykihthxwwqvx(double jrzpp, int ymfpugbqs, double uwqsvgnzbfsei, bool wodjinuxui, int awvsmj, double nqpoyjoho, double ekcnuhbzsijup, int ndyxzchbrtvug);

};



string fjjrzng::npwvvcsyndwrkf(double gefsoavizd, int ruhkpm, string wxbralghoy, double irfvfp, int mikaviksetg, int wzkzoxds, double djjofngf, bool goggzdmc, string ijqolcnznsv, bool tbvvg) {
	bool xghstmwqacqjj = true;
	if (true != true) {
		int nuuor;
		for (nuuor = 89; nuuor > 0; nuuor--) {
			continue;
		}
	}
	if (true != true) {
		int kk;
		for (kk = 46; kk > 0; kk--) {
			continue;
		}
	}
	return string("lmrvvicjv");
}

double fjjrzng::eoawmcarcjddzpxsskymkkv(double wsujxqqwkjytqq, int bmzupysnpormw, double txkbwoekgwb, int xtvfxg, int zxypy, int fyngcivxfmdqq, int gxfniurxjsy, double hxxoojs, int qqpcpmgm, int zierrvnzrcib) {
	double vnkoujz = 41346;
	string nrlendxsgpvzwxt = "nocqmtiawwhqvbjyplbcgrtfiulireujfxlmyrgbvsvflxbjargamitsjpijplxeukqwwtwyuhwkxeyyrmogqavubdfhf";
	string uxhhd = "jtlsrvfwdflmudovdkjhlssyqpbrvmbxrfthyatlctdyvzadhsc";
	if (string("jtlsrvfwdflmudovdkjhlssyqpbrvmbxrfthyatlctdyvzadhsc") != string("jtlsrvfwdflmudovdkjhlssyqpbrvmbxrfthyatlctdyvzadhsc")) {
		int tuebsx;
		for (tuebsx = 31; tuebsx > 0; tuebsx--) {
			continue;
		}
	}
	if (string("jtlsrvfwdflmudovdkjhlssyqpbrvmbxrfthyatlctdyvzadhsc") == string("jtlsrvfwdflmudovdkjhlssyqpbrvmbxrfthyatlctdyvzadhsc")) {
		int agospsu;
		for (agospsu = 46; agospsu > 0; agospsu--) {
			continue;
		}
	}
	if (string("jtlsrvfwdflmudovdkjhlssyqpbrvmbxrfthyatlctdyvzadhsc") == string("jtlsrvfwdflmudovdkjhlssyqpbrvmbxrfthyatlctdyvzadhsc")) {
		int bew;
		for (bew = 23; bew > 0; bew--) {
			continue;
		}
	}
	return 67689;
}

void fjjrzng::ccfizbjzrf(double ephlvvqdfsfbjgt, bool ovynztt, int vpkxe, bool egykre, int acseftpgdrn, double fyiayft, bool widgdoiyljup, bool roxfwtbn) {
	string dljlcopsu = "akslzfmibhbgdxoqdjnkn";
	string juqzc = "afqcbilxdbhrvlpdepvcutkxmjhpqxvaoxjvpyerdycbdygftdwcvfbpprtvgt";
	int jecwwm = 1866;
	bool matzisxygusa = true;
	if (true == true) {
		int zx;
		for (zx = 45; zx > 0; zx--) {
			continue;
		}
	}
	if (string("akslzfmibhbgdxoqdjnkn") == string("akslzfmibhbgdxoqdjnkn")) {
		int ouygqgc;
		for (ouygqgc = 27; ouygqgc > 0; ouygqgc--) {
			continue;
		}
	}
	if (string("afqcbilxdbhrvlpdepvcutkxmjhpqxvaoxjvpyerdycbdygftdwcvfbpprtvgt") != string("afqcbilxdbhrvlpdepvcutkxmjhpqxvaoxjvpyerdycbdygftdwcvfbpprtvgt")) {
		int omrto;
		for (omrto = 60; omrto > 0; omrto--) {
			continue;
		}
	}

}

int fjjrzng::fotgdfbimdnmfxvanft(double lvrskdas, double efgxdnpxnlqwap, bool qfbgqpzyc, double dhvszrimqbkp) {
	int nwyfnvcu = 2457;
	int xrxncrfzb = 2162;
	int arxenieughroadt = 7494;
	bool jaiqibkeobep = true;
	string mkqzhrpnut = "xyzbfqadjsqrlsqvthkjqatunctpfdykqbnfwawitegyaimhqafwpxmvanxmyyuenudevqaxgennxzwjphuvyjfhvmkbr";
	bool yldvztqg = true;
	string giyul = "aylgfzimopzflpilbhdzamjegcdhvfehozxuyxevmajalecbehj";
	if (true != true) {
		int rpobmfyap;
		for (rpobmfyap = 0; rpobmfyap > 0; rpobmfyap--) {
			continue;
		}
	}
	if (7494 != 7494) {
		int wzpratqfg;
		for (wzpratqfg = 83; wzpratqfg > 0; wzpratqfg--) {
			continue;
		}
	}
	if (true == true) {
		int dtvwawc;
		for (dtvwawc = 47; dtvwawc > 0; dtvwawc--) {
			continue;
		}
	}
	return 745;
}

string fjjrzng::jpwoddsolwy(string gtkfmv, bool ljltmhed, double mrthtcokc, double jwjqenvdtxcc, bool tuwnugto, string rssyt, double lcwrxnkutdfhyx, int tftsjwrxzcj) {
	string ybbtejdjq = "nuyujvpctifjamffarzh";
	double ybnst = 63093;
	string dyyvagd = "ynyegipxxbn";
	bool zdnugctlglzatx = false;
	bool zqitvjgd = true;
	double yqdfme = 70377;
	int wjourrdu = 6328;
	if (true == true) {
		int dklnwnupbv;
		for (dklnwnupbv = 96; dklnwnupbv > 0; dklnwnupbv--) {
			continue;
		}
	}
	return string("g");
}

void fjjrzng::dgwvftcykihthxwwqvx(double jrzpp, int ymfpugbqs, double uwqsvgnzbfsei, bool wodjinuxui, int awvsmj, double nqpoyjoho, double ekcnuhbzsijup, int ndyxzchbrtvug) {
	bool urasmpfostpu = false;
	int ifyaflumvbgrdbl = 5116;
	int nvhqdadjkjbdqw = 5292;
	string exjjfzakv = "anmyek";
	double kcntcormh = 10401;
	string jjhem = "owpiocgpgforeiigmjcbxtauqgvpwbkvlcorixzxhmehviyguctxdrefxfidbiqosxncjubntnoenfsypxiocnhygn";

}

double fjjrzng::zzwpogoxrcqsdgho(bool jyascudmedno, string efuipwd, bool ogojovvnpyzmmi, string jxlumykiy, int roadboxpned, int ebrgbiadfw, bool ypjtjiryt, string rmujjwxtttxmfwy, int jzletvsyldiyqi, double jfpzcstqk) {
	int efmmk = 1508;
	if (1508 == 1508) {
		int rldixhirv;
		for (rldixhirv = 8; rldixhirv > 0; rldixhirv--) {
			continue;
		}
	}
	if (1508 != 1508) {
		int veuwybjuzu;
		for (veuwybjuzu = 64; veuwybjuzu > 0; veuwybjuzu--) {
			continue;
		}
	}
	return 50998;
}

void fjjrzng::loksyivwxgennnscjfgv(double xzxcboduyno, string vqraufblfxnnuqd) {

}

void fjjrzng::tkvzqzfwtfpzuw() {
	bool twnghhtwmerzyx = false;
	double vmpthufk = 6221;
	double ckxekbwyd = 33311;
	double kntndejaemsxjo = 19699;
	double ukimkrteca = 15941;
	int hvzeblwtn = 8434;
	int qrhuglzcfxdjtl = 432;
	string evnaqblcqqkfh = "xbyt";
	bool tdcduabxbitjzwc = true;
	if (string("xbyt") == string("xbyt")) {
		int uvm;
		for (uvm = 67; uvm > 0; uvm--) {
			continue;
		}
	}

}

bool fjjrzng::jytmemzmlnydy(int qlnciuhxkehp, string rcsyqgon) {
	bool bxqlp = true;
	bool cljlh = true;
	double nmikqxmwdhrp = 43299;
	int bjfqolrwzsyri = 1750;
	int aygxwwvfm = 905;
	bool nypwavdvahb = true;
	int msrsfubja = 541;
	string qnbssiskxpw = "jylybqcwszmgjhdbbvnxcezcpfyhvigjyvtvlbxrdnfviniz";
	int brschuppj = 6013;
	if (6013 != 6013) {
		int oaronzgdv;
		for (oaronzgdv = 36; oaronzgdv > 0; oaronzgdv--) {
			continue;
		}
	}
	if (true == true) {
		int sk;
		for (sk = 30; sk > 0; sk--) {
			continue;
		}
	}
	if (string("jylybqcwszmgjhdbbvnxcezcpfyhvigjyvtvlbxrdnfviniz") == string("jylybqcwszmgjhdbbvnxcezcpfyhvigjyvtvlbxrdnfviniz")) {
		int cgq;
		for (cgq = 7; cgq > 0; cgq--) {
			continue;
		}
	}
	if (string("jylybqcwszmgjhdbbvnxcezcpfyhvigjyvtvlbxrdnfviniz") == string("jylybqcwszmgjhdbbvnxcezcpfyhvigjyvtvlbxrdnfviniz")) {
		int lram;
		for (lram = 9; lram > 0; lram--) {
			continue;
		}
	}
	return false;
}

string fjjrzng::qgwfkyyigvfaaz(double yhqvdbptwmr, double ialbxllzwyaw, bool cwwsmsokyyriq) {
	string tfrkmebytlmoskw = "imcodhuqwk";
	int bavfihadqrkvmh = 1725;
	return string("wyvlxprkqbme");
}

string fjjrzng::dwsyytpibtyxcxdpxojhxy() {
	double ebzvxszr = 84187;
	string cyanljkushphdn = "gdbbdbdgndpzwlxctlqfrltyymhirufptfipvykmbtyhsquohiq";
	double nshqqftiip = 12238;
	int xknjnygyft = 1834;
	int wiaakndag = 1809;
	string cpfyvdvgtli = "mhvojhnfdeuxwdmlfscqxjygrwsykqphdwydudxrhncnnbluqmypkurtoyreffmzmdqisceycpabhpaqxuzjjtsbce";
	bool pakydngtns = true;
	bool kyxxoefkpxixyck = false;
	string kkmudj = "gbpekl";
	if (false != false) {
		int ifmki;
		for (ifmki = 21; ifmki > 0; ifmki--) {
			continue;
		}
	}
	if (1809 != 1809) {
		int cfrbsg;
		for (cfrbsg = 82; cfrbsg > 0; cfrbsg--) {
			continue;
		}
	}
	if (false == false) {
		int atfhzbj;
		for (atfhzbj = 38; atfhzbj > 0; atfhzbj--) {
			continue;
		}
	}
	return string("nayvp");
}

int fjjrzng::gcuqixicntnwfi(double qpfvaxhmaqartw, string cahsicizyjoybd) {
	int cwdscnaonr = 128;
	string psejcob = "ttzivdbzscpftwmyngdckfmevlupsgrjqmrpaxdmvuuzvtikwyqvrocj";
	double xweteabhmt = 3928;
	int iijppbainpf = 1824;
	bool vgebczkabbaet = true;
	string ktsdhlge = "ahhdwyrwbpdgxzrampfelapxd";
	return 9093;
}

double fjjrzng::zuzessmefdp(string kmkqgaiaarv) {
	double pptgppjitj = 97812;
	double krbxuscibz = 18046;
	bool kdmahkuiefcy = true;
	bool xspgbuxgr = true;
	if (true == true) {
		int bligagw;
		for (bligagw = 26; bligagw > 0; bligagw--) {
			continue;
		}
	}
	return 7905;
}

void fjjrzng::msgwdcrgufoh(bool hlgjkm, bool blscfpnmrapy, string tlmkiy, int bosazaaesqhlcdx, string ixvvbyhjwg, int ecpnareggsdb, string ilbofq, string inxximsmiucxbxy) {
	int fgwvhomzzow = 5366;
	int wufjawrtkobytkx = 267;
	int rrfcsxjpdraa = 1139;
	int xwcqkuvvhmljpc = 23;
	double vvrzjdyakngd = 11196;

}

int fjjrzng::eohvawllrjcwiuaumzniqf(bool pcpwzu, string cgyypciub, int flpylw, int ptmumywurhonlvh, string uafisdoqrapze, double pfzffnio, string yxdbuinmnsqlpqe, double oqdnksowhru) {
	bool qeoce = true;
	if (true == true) {
		int sqwo;
		for (sqwo = 42; sqwo > 0; sqwo--) {
			continue;
		}
	}
	if (true == true) {
		int xwcybsh;
		for (xwcybsh = 26; xwcybsh > 0; xwcybsh--) {
			continue;
		}
	}
	return 29897;
}

fjjrzng::fjjrzng() {
	this->tkvzqzfwtfpzuw();
	this->jytmemzmlnydy(1652, string("cfhhcdzyvevyxhrnwgdveivbykolovnz"));
	this->qgwfkyyigvfaaz(49084, 10988, true);
	this->dwsyytpibtyxcxdpxojhxy();
	this->gcuqixicntnwfi(4676, string("abjpialcxeotsppkiyomfnmelxhcdvgbwjxbxeeznpnrjwpovptjtnyrkcltmcdvsg"));
	this->zuzessmefdp(string("seenvsifnubvmwmnmlsxbwlht"));
	this->msgwdcrgufoh(true, true, string("wljckxryvhuvefgxdfyezad"), 8124, string("lbjojxherxswmoftjmlyfgnupkuxvgrvebzqezq"), 3005, string("fdmhzdwssoahokiykfiygvvhcfaajftgyycllndkciwezcwquwtiqbrojlwntlxlzpdijvehptysydqmngvabevhfncpzzw"), string("pwnkbxmvxrtviyzelpzzqrnpfrtgojikfunkylajmtmhnlfdoollutozmiupxqxscsg"));
	this->eohvawllrjcwiuaumzniqf(true, string("eefcwyzbdfrrfoydjzllvvsdqxvyesujddmfbfanhjwhnmavsuzbwdglrsuqvs"), 891, 431, string("qtsxoyzkprayaryswsliwiofglcuohzdvenntbgdgoeqrnxcaxbpnhklbyvwembkmmdwtkamlq"), 7660, string("mlwtmktxfzcngcdrnnpkapbkgtvvbsnqezngexxqcibk"), 11076);
	this->zzwpogoxrcqsdgho(true, string("mgkfysmwohwqinscieuisfxgjmfnfnbdlmneraib"), false, string("snuzokljcziakxrbijubuzcxgddgdyybrlfjfcxvdsjwtgmrqeeeillhoutynvvmpokhajgonirtrctigm"), 8023, 4420, false, string("kueejmyblepnjilxqdsrazqurywjqmqjzxoswsspscwwqowobvyurkbmakqrwvguezrzxwnrvvt"), 2474, 11587);
	this->loksyivwxgennnscjfgv(62816, string("boqxqukqlvuuewfdscbkjlsytbxyumdujfwbrhkdgzzbutldvc"));
	this->npwvvcsyndwrkf(10137, 7552, string("ujanzkisrygzqgbbknewitnikdzhleptrawqkijjfmufpwiplxdqjnfiucvbqftsl"), 76580, 824, 297, 10381, false, string("ttngdysdrjenggoxjjvqqxnadiklyaejrwzlcyherdl"), false);
	this->eoawmcarcjddzpxsskymkkv(7612, 234, 4482, 7100, 2070, 1374, 1158, 55448, 1545, 5535);
	this->ccfizbjzrf(8487, true, 2502, true, 3947, 21964, false, true);
	this->fotgdfbimdnmfxvanft(71761, 18740, false, 13528);
	this->jpwoddsolwy(string("q"), true, 66738, 43517, true, string("ybdsfmuewzfzlazzuwsdfo"), 20685, 412);
	this->dgwvftcykihthxwwqvx(13627, 442, 27989, true, 2059, 10727, 45719, 8572);
}

int CRageBot::FindBestEntity(CUserCmd* cmd, C_BaseCombatWeapon* weapon, Vector& hitpos, bool& bBacktrack)
{
	int BestEntityIndex = -1;
	float WeaponRange = weapon->GetCSWeaponData()->flRange;
	float minFoV = (float)Vars.ragebot_fov;
	
	Vector ViewOffset = Globals::LocalPlayer->m_vecOrigin() + Globals::LocalPlayer->m_vecViewOffset();
	QAngle view; Interfaces::Engine->GetViewAngles(view);
	float BestDamage = 0.f;
	Vector BestHitpoint;
	bool BestBacktrack = false;

	for (int i = 0; i <= Interfaces::Engine->GetMaxClients(); i++)
	{
		auto entity = (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(i);

		if (!entity)
			continue;

		if (!Globals::LocalPlayer)
			continue;

		if (!entity->IsPlayer())
			continue;

		if (!entity->IsAlive())
			continue;

		if (entity->IsDormant())
			continue;

		if (entity->IsTeammate()) 
			continue;

		if (entity->m_bGunGameImmunity()) 
			continue;

		if (entity->IsNotTarget())
			continue;

		if (!matrixData[i].gotMatrix)
			continue;

		float Distance = Math::VectorDistance(Globals::LocalPlayer->GetEyePos(), entity->GetEyePos());

		if (Distance > WeaponRange)
			continue;

		float CDamage = 0.f;
		Vector CHitpos;
		bool CUsingBacktrack = false;
		
		bool WillKillEntity = false;

		if (!GetBestHitboxPoint(entity, CDamage, CHitpos, (BaimMode)0, WillKillEntity))
			continue;

		QAngle viewAngles;
		Interfaces::Engine->GetViewAngles(viewAngles);

		float fov = Math::GetFOV(viewAngles, Math::CalcAngle(Globals::LocalPlayer->GetEyePos(), CHitpos));

		if (fov > Vars.ragebot_fov)
			continue;

		switch (Vars.ragebot_selection[curGroup])
		{
		case 0:
			if (fov < minFoV)
			{
				minFoV = fov;
				BestHitpoint = CHitpos;
				BestEntityIndex = entity->EntIndex();
			}
			break;
		case 1:
			if (CDamage > BestDamage)
			{
				BestDamage = CDamage;
				BestHitpoint = CHitpos;
				BestEntityIndex = entity->EntIndex();
			}
			break;
		}
	}

	hitpos = BestHitpoint;
	return BestEntityIndex;
}

bool CRageBot::GetBestHitboxPoint(C_BasePlayer* entity, float& damage, Vector& hitbox, BaimMode baim, bool& WillKill, matrix3x4_t matrix[MAXSTUDIOBONES], mstudiohitboxset_t* StudioSet, bool NoPointscale)
{
	matrix3x4_t cmatrix[MAXSTUDIOBONES];

	if (!matrix)
	{
		matrix = matrixData[entity->EntIndex()].matrix;
	}

	if (!StudioSet)
		StudioSet = matrixData[entity->EntIndex()].StudioSet;

	WillKill = false;

	float BestDamage = 0.f;
	Vector BestHitpoint;
	bool FoundHitableEntity = false;

	CanHitStruct CanHitHead;
	CanHitStruct CanBaimKill;

	for (int hitbox = 0; hitbox < Hitboxes::HITBOX_MAX; hitbox++)
	{
		if ((hitbox == HITBOX_HEAD || hitbox == HITBOX_NECK || hitbox >= HITBOX_RIGHT_THIGH) && baim == BaimMode::FORCE_BAIM)
			continue;

		float pointscale = 0.f;

		switch (hitbox)
		{
		case HITBOX_HEAD:
			if (!Vars.ragebot_hitbox[0][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[0][curGroup];
			break;

		case HITBOX_NECK:
			if (!Vars.ragebot_hitbox[1][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[1][curGroup];
			break;
		case HITBOX_PELVIS:
			if (!Vars.ragebot_hitbox[2][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[2][curGroup];
			break;
		case HITBOX_STOMACH:
			if (!Vars.ragebot_hitbox[3][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[3][curGroup];
			break;
		case HITBOX_LOWER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			if (!Vars.ragebot_hitbox[4][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[4][curGroup];
			break;
		case HITBOX_RIGHT_THIGH:
		case HITBOX_LEFT_THIGH:
		case HITBOX_RIGHT_CALF:
		case HITBOX_LEFT_CALF:
			if (!Vars.ragebot_hitbox[5][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[5][curGroup];
			break;

		case HITBOX_RIGHT_FOOT:
		case HITBOX_LEFT_FOOT:
			if (!Vars.ragebot_hitbox[6][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[6][curGroup];
			break;
		case HITBOX_RIGHT_HAND:
		case HITBOX_LEFT_HAND:
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
			if (!Vars.ragebot_hitbox[7][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[7][curGroup];
			break;
		default:
			continue;
		}

		if (NoPointscale)
			pointscale = 0.f;

		std::vector<PointScanStruct> Points = GetPointsForScan(entity, hitbox, StudioSet, matrix, pointscale);

		float CBestDamage = 0.f;
		Vector CBestPoint;
		Vector CCenter;
		bool CCanHitCenter = false;
		float CCenterDamage = -1.f;

		for (size_t p = 0; p < Points.size(); p++)
		{
			float CDamage = 0.f;
			CDamage = CAutoWall::Get().CanHit(Points[p].pos);

			if ((hitbox == HITBOX_HEAD || hitbox == HITBOX_NECK) && baim == BaimMode::BAIM)
			{
				if (CDamage >= Vars.ragebot_mindamage[curGroup] && CDamage > CanHitHead.damage)
				{
					CanHitHead.CanHit = true;
					CanHitHead.damage = CDamage;
					CanHitHead.pos = Points[p].pos;
				}

				continue;
			}

			if (baim == BaimMode::BAIM && (hitbox >= HITBOX_RIGHT_HAND || hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH || hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF))
				continue;

			if (Points[p].center && CDamage >= Vars.ragebot_mindamage[curGroup])
			{
				CCanHitCenter = true;
				CCenter = Points[p].pos;
				CCenterDamage = CDamage;
			}


			if (CDamage >= Vars.ragebot_mindamage[curGroup] && CDamage > CBestDamage)
			{
				CBestDamage = CDamage;
				CBestPoint = Points[p].pos;
			}
		}

		if (CCanHitCenter && CCenterDamage >= entity->m_iHealth() / 2.f)
		{
			CBestDamage = CCenterDamage;
			CBestPoint = CCenter;
		}

		if (CBestDamage >= Vars.ragebot_mindamage[curGroup] && CanBaimKill.damage < CBestDamage && (hitbox == HITBOX_CHEST || hitbox == HITBOX_LOWER_CHEST || hitbox == HITBOX_PELVIS || hitbox == HITBOX_STOMACH))
		{
			CanBaimKill.CanHit = true;
			CanBaimKill.damage = CBestDamage;
			CanBaimKill.pos = CBestPoint;
		}


		if (CBestDamage >= Vars.ragebot_mindamage[curGroup] && CBestDamage > BestDamage)
		{
			BestDamage = CBestDamage;
			BestHitpoint = CBestPoint;
			FoundHitableEntity = true;
		}

	}

	if (!FoundHitableEntity && CanHitHead.CanHit)
	{
		FoundHitableEntity = true;
		BestDamage = CanHitHead.damage;
		BestHitpoint = CanHitHead.pos;
	}

	if (CanBaimKill.damage >= entity->m_iHealth())
	{
		FoundHitableEntity = true;
		BestDamage = CanBaimKill.damage;
		BestHitpoint = CanBaimKill.pos;
	}

	if (BestDamage >= entity->m_iHealth())
		WillKill = true;

	damage = BestDamage;
	hitbox = BestHitpoint;

	return FoundHitableEntity;
}

int total_missed[64];
int total_hit[64];
IGameEvent* event = nullptr;
extra s_extra;

void CRageBot::missed_due_to_desync(IGameEvent* event)
{
	if (event == nullptr)
		return;

	int user = event->GetInt("userid");
	int attacker = event->GetInt("attacker");
	bool player_hurt[64], hit_entity[64];

	if (Interfaces::Engine->GetPlayerForUserID(user) != Interfaces::Engine->GetLocalPlayer()
		&& Interfaces::Engine->GetPlayerForUserID(attacker) == Interfaces::Engine->GetLocalPlayer()) {
		player_hurt[Interfaces::Engine->GetPlayerForUserID(user)] = true;
	}

	if (Interfaces::Engine->GetPlayerForUserID(user) != Interfaces::Engine->GetLocalPlayer())
	{
		Vector bullet_impact_location = Vector(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));
		if (Globals::aim_point != bullet_impact_location) return;
		hit_entity[Interfaces::Engine->GetPlayerForUserID(user)] = true;
	}
	if (!player_hurt[Interfaces::Engine->GetPlayerForUserID(user)] && hit_entity[Interfaces::Engine->GetPlayerForUserID(user)]) {
		s_extra.current_flag[Interfaces::Engine->GetPlayerForUserID(user)] = correction_flags::DESYNC;
		++total_missed[Interfaces::Engine->GetPlayerForUserID(user)];
	}
	if (player_hurt[Interfaces::Engine->GetPlayerForUserID(user)] && hit_entity[Interfaces::Engine->GetPlayerForUserID(user)]) {
		++total_hit[Interfaces::Engine->GetPlayerForUserID(user)];
	}
}

float randnum(int Min, int Max)
{
	return ((rand() % (Max - Min)) + Min);
}

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif


#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

float GetDesyncDelta(C_BasePlayer* entity)
{
	auto animstate = uintptr_t(entity->GetBasePlayerAnimState());
	float duckammount = *(float *)(animstate + 0xA4);
	float speedfraction = max(0, min(*reinterpret_cast< float* >(animstate + 0xF8), 1));
	float speedfactor = max(0, min(1, *reinterpret_cast< float* > (animstate + 0xFC)));
	float unk1 = ((*reinterpret_cast< float* > (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
	float unk2 = unk1 + 1.f;
	float unk3;

	if (duckammount > 0)
	{
		unk2 += ((duckammount * speedfactor) * (0.5f - unk2));
	}
	unk3 = *(float *)(animstate + 0x334) * unk2;
	return unk3 + randnum(-10, 0);
}

void CRageBot::Resolver(CUserCmd *cmd, C_BasePlayer* entity)
{
	if (!Vars.ragebot_resolver)
		return;

	if (!Utils::IsInGame())
		return;

	if (!Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive())
		return;

	missed_due_to_desync(event);

	bool jitter_desync[64] = { false };;

	float old_yaw[64] = { NULL };
	float current_yaw[64] = { NULL };
	float oldlby[64] = { NULL };
	float test_yaw[64] = { NULL };
	float brutelby[64] = { NULL };
	float test[64] = { NULL };
	float resolved_yaw[64] = { 0.f };
	float b1gdelta[64], lbydelta[64] = { 0.f };

	auto feet_yaw = entity->GetBasePlayerAnimState()->m_flCurrentFeetYaw;
	auto body_max_rotation = entity->GetBasePlayerAnimState()->pad10[516];

	if (entity->m_vecVelocity().Length2D() > 0.1 && entity->m_vecVelocity().Length2D() <= 40.f)
	{
		switch (total_missed[entity->EntIndex()])
		{
			// don't do anything if we missed 0 times
		case 1: entity->m_angEyeAngles().yaw = entity->m_flLowerBodyYawTarget();
			break;
		case 2: body_max_rotation + entity->m_angEyeAngles().yaw;
			break;
		case 3: body_max_rotation - entity->m_angEyeAngles().yaw;
			break;
		case 4:entity->m_angEyeAngles().yaw = entity->m_flLowerBodyYawTarget() + 29;
			break;
		}
	}

	if (entity->m_vecVelocity().Length2D() <= 0.1)
	{
		if (feet_yaw <= 58.f)
		{
			if (feet_yaw < -58.f)
				entity->m_angEyeAngles().yaw = body_max_rotation + entity->m_angEyeAngles().yaw;
		}
	}
}

void CRageBot::PVSFix(ClientFrameStage_t stage)
{
	if (stage != FRAME_RENDER_START)
		return;

	for (int i = 1; i <= Interfaces::GlobalVars->maxClients; i++)
	{
		if (i == Interfaces::Engine->GetLocalPlayer()) 
			continue;

		IClientEntity* pCurEntity = Interfaces::EntityList->GetClientEntity(i);
		if (!pCurEntity) continue;

		*(int*)((uintptr_t)pCurEntity + 0xA30) = Interfaces::GlobalVars->framecount; //we'll skip occlusion checks now
		*(int*)((uintptr_t)pCurEntity + 0xA28) = 0;//clear occlusion flags
	}
}

void CRageBot::PrecacheShit()
{
	if (!Vars.ragebot_enabled)
		return;

	for (int i = 0; i <= Interfaces::Engine->GetMaxClients(); i++)
	{
		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!entity) 
			continue;

		if (!Globals::LocalPlayer) 
			continue;

		if (!entity->IsPlayer())
			continue;

		if (!entity->IsAlive())
			continue;

		if (entity->IsDormant())
			continue;

		if (entity->IsTeammate())
			continue;

		model_t* pModel = entity->GetModel();

		if (!pModel)
		{
			matrixData[i].gotMatrix = false;
			continue;
		}

		matrixData[i].StudioHdr = g_MdlInfo->GetStudiomodel(pModel);

		if (!matrixData[i].StudioHdr)
		{
			matrixData[i].gotMatrix = false;
			continue;
		}

		matrixData[i].StudioSet = matrixData[i].StudioHdr->GetHitboxSet(0);

		if (!matrixData[i].StudioSet)
		{
			matrixData[i].gotMatrix = false;
			continue;
		}

		matrixData[i].gotMatrix = entity->SetupBones(matrixData[i].matrix, 128, 256, entity->m_flSimulationTime());
	}
}

std::vector<PointScanStruct> CRageBot::GetPointsForScan(C_BasePlayer* entity, int hitbox, mstudiohitboxset_t* hitset, matrix3x4_t matrix[MAXSTUDIOBONES], float pointscale)
{
	std::vector<PointScanStruct> pointsToScan;

	if (!matrix)
		return pointsToScan;

	if (!hitset)
		return pointsToScan;

	mstudiobbox_t* bbox = hitset->GetHitbox(hitbox);

	if (!bbox)
		return pointsToScan;

	float mod = bbox->m_flRadius != -1.f ? bbox->m_flRadius : 0.f;

	Vector max;
	Vector min;

	Vector in1 = bbox->bbmax + mod;
	Vector in2 = bbox->bbmin - mod;


	Math::VectorTransform(in1, matrix[bbox->bone], max);
	Math::VectorTransform(in2, matrix[bbox->bone], min);

	Vector center = (min + max) * 0.5f;

	QAngle curAngles = Math::CalcAngle(center, Globals::LocalPlayer->GetEyePos());

	Vector forward;
	Math::AngleVectors(curAngles, forward);

	Vector right = forward.Cross(Vector(0, 0, 1));
	Vector left = Vector(-right.x, -right.y, right.z);

	Vector top = Vector(0, 0, 1);
	Vector bot = Vector(0, 0, -1);

	if (pointscale == 0.f)
	{
		pointsToScan.emplace_back(PointScanStruct{ center, true });
		return pointsToScan;
	}

	if (hitbox == HITBOX_HEAD)
	{
		for (auto i = 0; i < 5; ++i)
		{
			pointsToScan.emplace_back(PointScanStruct{ center });
		}

		pointsToScan[1].pos += top * (bbox->m_flRadius * pointscale);
		pointsToScan[2].pos += right * (bbox->m_flRadius * pointscale);
		pointsToScan[3].pos += left * (bbox->m_flRadius * pointscale);
		pointsToScan[4].pos = center;
		pointsToScan[4].center = true;
	}
	else
	{
		for (auto i = 0; i < 3; ++i)
		{
			pointsToScan.emplace_back(PointScanStruct{ center });
		}

		pointsToScan[0].pos += right * (bbox->m_flRadius * pointscale);
		pointsToScan[1].pos += left * (bbox->m_flRadius * pointscale);
		pointsToScan[2].pos = center;
		pointsToScan[2].center = true;
	}

	return pointsToScan;
}