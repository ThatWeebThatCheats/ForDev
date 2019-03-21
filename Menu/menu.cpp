#include <chrono>
#include <D3dx9tex.h>
#include "menu.hpp"

#include "../arrays.h"

#include "../Valve_SDK/csgostructs.hpp"
#include "../Helpers/input.hpp"
#include "../options.hpp"
#include "ui.hpp"
#include "../fonts.h"
#include "../ImGui/imgui_internal.h"
#include "../ImGui/directx9/imgui_impl_dx9.h"
#include "../Features/misc/misc.hpp"
#include "../Features/skinchanger/skins.hpp"
#include "../Features/visuals/nightmode.hpp"
#include "../Helpers/config_manager.hpp"
#include "../Valve_SDK/interfaces/Logger.h"
#include "Header.h"

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

ImFont* bigFont = nullptr;
ImFont* forTabs = nullptr;
ImFont* forTabs_text = nullptr;
ImFont* forTabs_text_x2 = nullptr;
ImFont* forTabs_text_x22 = nullptr;

ImFont* globalFont = nullptr;
WNDPROC original_proc = nullptr;
static int tabselected = 0;
IMGUI_API LRESULT   ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool isSkeet = false;

#define AIMBOT XorStr("A")
#define RAGE XorStr("K")
#define VISUALS XorStr("D")
#define SKINS XorStr("B")
#define COLORS XorStr("H")
#define MISC XorStr("G")
#define AA XorStr("K")

static int a1 = 0;

void CMenu::ImColors(ImGuiStyle& style)
{
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 0.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.f, 1.0f, 0.f, 1.f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.f, 1.0f, 0.f, 1.f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.59f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.f, 1.f, 0.f, 1.f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);
}

void DefaultColor()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

class nujrfez {
public:
	double bzpfppt;
	string opbwfq;
	string sbibpnp;
	int nbrzp;
	double cvkvqomb;
	nujrfez();
	string dcwdxtmeuniswoif(int bprmf, int exbcvfdtokaoqf, double yqfjnnylpbs);
	int lysovkgyjg(double wqgdnny, string gfktrxasw, int nsgce, string sjseg, bool mgrbgpbgeitdgpm, double rartd, int attlpzmeny, double iwmkjegxpyo);
	int xlgzzwbapyicnqo(int dfmjlyfyeibc, double vbkcndyaycg, bool fbcxxg, int cziqqp, string gfdpgxehtdxh, string ierrczqfz, double ddnthb, bool yvqbilp);
	void qnauhkyatpwgutlya(string dtlocfumr, string jqjvrmogcz, string bqvoasitrbbi, int raqcdoiuencfz, string ilrnajjl, string gvjiccouaywxjxh, int kapojrqlirsaa, bool ytlrobhy, double anutfulgvvthks, string iusxjgk);

protected:
	string otszyonkwiiski;
	int jtjxfo;
	string menel;
	string zsysltpzrmdhqdt;

	double xwapkpiuvlkqgzhixvw(double cuesuccoy);

private:
	double upgkkovprmef;
	double wwigeui;

	void woylvclina(int uyevmmtdfbztles, int baghggskglisa, string rwxiqjd);

};



void nujrfez::woylvclina(int uyevmmtdfbztles, int baghggskglisa, string rwxiqjd) {
	string hftoiaqvtulh = "xbifvckhcncfprvsaaubphadosapjumwsbbbxyrnbqbvobqtjzwoaraqpaxffxnlbzrzowhjwvy";
	string vwfhcpzukoe = "vhkhyipoga";
	double xznbg = 4177;

}

double nujrfez::xwapkpiuvlkqgzhixvw(double cuesuccoy) {
	bool zmrwfwhazimcb = true;
	string idgjusulpvecpjo = "xxodyxxvbpbrdxzqqtpmnsqwfqnxkbnebgjefdqjlisrrmhwnhueeezaonepmi";
	bool evnrptezjqcgad = true;
	double umsbadm = 18141;
	string bqhrhlnotpnzvrj = "ehxdgymjnacgdvxuhbxklkftfsyerjfpl";
	bool uyfewnb = true;
	bool prxtekx = false;
	int grgfhz = 452;
	string vuyqirnlzpobrj = "eerbdpickkzchaduybpgofefxqxzfcjnmgpakityhzaefwkwezikxmumvjlpqptbzvfkfniiuzoowkzqqvfnac";
	double jmxnrwpzksdl = 36031;
	if (18141 == 18141) {
		int mzudhnb;
		for (mzudhnb = 82; mzudhnb > 0; mzudhnb--) {
			continue;
		}
	}
	if (true != true) {
		int bw;
		for (bw = 74; bw > 0; bw--) {
			continue;
		}
	}
	return 68312;
}

string nujrfez::dcwdxtmeuniswoif(int bprmf, int exbcvfdtokaoqf, double yqfjnnylpbs) {
	string oqvpw = "rzwwfjgneixtrxwvdqtniegvve";
	int fdcji = 610;
	double vxnhdikdfcn = 16085;
	string yoqtrlskten = "qllthkgofxteanlkjpmtnxfmonmbwequemgzbqk";
	bool tdvppmp = false;
	int eyyicgqf = 418;
	int pqydzhblqxbcgei = 939;
	bool xvgoqucow = true;
	string njocgvkmvokg = "ubkausttskotjxcmuavdxqdlknkqpvpxmcrmnhnwrwblmwhuvyy";
	if (string("qllthkgofxteanlkjpmtnxfmonmbwequemgzbqk") != string("qllthkgofxteanlkjpmtnxfmonmbwequemgzbqk")) {
		int ubtnksdo;
		for (ubtnksdo = 12; ubtnksdo > 0; ubtnksdo--) {
			continue;
		}
	}
	if (false == false) {
		int obpkcbb;
		for (obpkcbb = 37; obpkcbb > 0; obpkcbb--) {
			continue;
		}
	}
	if (418 == 418) {
		int sintloj;
		for (sintloj = 87; sintloj > 0; sintloj--) {
			continue;
		}
	}
	if (string("ubkausttskotjxcmuavdxqdlknkqpvpxmcrmnhnwrwblmwhuvyy") != string("ubkausttskotjxcmuavdxqdlknkqpvpxmcrmnhnwrwblmwhuvyy")) {
		int lj;
		for (lj = 24; lj > 0; lj--) {
			continue;
		}
	}
	if (false != false) {
		int ecgthktyv;
		for (ecgthktyv = 47; ecgthktyv > 0; ecgthktyv--) {
			continue;
		}
	}
	return string("oqmkbdkyepbjmqgktylu");
}

int nujrfez::lysovkgyjg(double wqgdnny, string gfktrxasw, int nsgce, string sjseg, bool mgrbgpbgeitdgpm, double rartd, int attlpzmeny, double iwmkjegxpyo) {
	string eurbueqkr = "ueltjmlwaogardzfytxqdcysocxlbjvtxidmrcqlqvkwryqpmtnnxjjyoingmoyrqyzsdnlilzvbctjdwn";
	string aileyqdn = "p";
	bool ocyopui = true;
	if (true != true) {
		int bfhyxgza;
		for (bfhyxgza = 49; bfhyxgza > 0; bfhyxgza--) {
			continue;
		}
	}
	return 99366;
}

int nujrfez::xlgzzwbapyicnqo(int dfmjlyfyeibc, double vbkcndyaycg, bool fbcxxg, int cziqqp, string gfdpgxehtdxh, string ierrczqfz, double ddnthb, bool yvqbilp) {
	bool jxbzsp = false;
	string iabyjawcamkzaa = "mpaicnrdfboiebdpbohbkzz";
	int fquyeyww = 372;
	string bnxpkyxvzrazfu = "awtvbrv";
	bool ktgnlscquud = true;
	int djesorlq = 552;
	bool zldesfqrdubxl = true;
	int jopiaygnw = 2513;
	int zdfkpfk = 758;
	int qzloskxmlvcdr = 2054;
	if (372 == 372) {
		int lsrazmfpy;
		for (lsrazmfpy = 54; lsrazmfpy > 0; lsrazmfpy--) {
			continue;
		}
	}
	if (string("awtvbrv") == string("awtvbrv")) {
		int fcyrhwn;
		for (fcyrhwn = 8; fcyrhwn > 0; fcyrhwn--) {
			continue;
		}
	}
	if (string("awtvbrv") == string("awtvbrv")) {
		int krhikc;
		for (krhikc = 70; krhikc > 0; krhikc--) {
			continue;
		}
	}
	return 8;
}

void nujrfez::qnauhkyatpwgutlya(string dtlocfumr, string jqjvrmogcz, string bqvoasitrbbi, int raqcdoiuencfz, string ilrnajjl, string gvjiccouaywxjxh, int kapojrqlirsaa, bool ytlrobhy, double anutfulgvvthks, string iusxjgk) {
	string scwffpwz = "qmuymtuccghdqoihkyrhtyxhzwmqbrcalqkrfxatjfwkqqegwjmbrdwjcjma";
	double rvweykbctom = 19011;
	double ibixnjo = 34534;
	string bcwqu = "cgtjsmaadyjebtzueochfhrmcuvwwhdcrklpuau";
	bool cewfttp = true;
	int bizfcfmzbwbz = 4059;
	double oopjrtmmb = 68344;
	if (true != true) {
		int bljvezssp;
		for (bljvezssp = 95; bljvezssp > 0; bljvezssp--) {
			continue;
		}
	}
	if (string("qmuymtuccghdqoihkyrhtyxhzwmqbrcalqkrfxatjfwkqqegwjmbrdwjcjma") == string("qmuymtuccghdqoihkyrhtyxhzwmqbrcalqkrfxatjfwkqqegwjmbrdwjcjma")) {
		int hcmopgjsdk;
		for (hcmopgjsdk = 35; hcmopgjsdk > 0; hcmopgjsdk--) {
			continue;
		}
	}
	if (4059 != 4059) {
		int ljxdmkm;
		for (ljxdmkm = 99; ljxdmkm > 0; ljxdmkm--) {
			continue;
		}
	}

}

nujrfez::nujrfez() {
	this->dcwdxtmeuniswoif(690, 5440, 13345);
	this->lysovkgyjg(38316, string("ispjcbywiawidomerqpsbsjcmkerjxhkiwofyuqyoacryuyrxznqosvatygoedammdskogoizzdcqqdnuhjoqwpisaabxavhznh"), 4394, string("ilhdhjntdcpdbhmgrwmehdhfhkgexzaddowfkqrec"), false, 16001, 303, 24479);
	this->xlgzzwbapyicnqo(1648, 2702, true, 7503, string("dlfhhjysxyhfkcpkffnfyantecwvsdxgvjkhdnticg"), string("vmlxouleiwnesyryvpitpufvftsnratdunekqvkzujofnsesluuukofuypdlrwzarmuktki"), 61726, false);
	this->qnauhkyatpwgutlya(string("xzzrwzqnbvipdlmwvnsamiohnqckeprgnc"), string("cooogiwakivftcf"), string("dsudshcdkwduscotlaviuytrfxjcitmiaxvrfqzjqubodwfvxwpeoq"), 1743, string("ftefyhzgftdcxkpecqehbhckddhwgdxfullxiqipqlfdoqagxwlpkpkqiefkvlbhctxebglhribfmwpcxxelglsomdeuemlgeh"), string("gwjpmmvofmehpfcudhslwpemszeajmctvbkqdiwxgkhgdvihyaxwqaepxtmudwzwwgulqixaipemysfbvcivnkg"), 4651, false, 10651, string("hnral"));
	this->xwapkpiuvlkqgzhixvw(51001);
	this->woylvclina(265, 2431, string("bxsskvyisggmkcmpvryddhckixfakxumiydnubuzyzmgurhpefkrxhpkgvpzwezxytgwxufbpcuvbosy"));
}



void CMenu::ImStyles(ImGuiStyle& style)
{//
	style.Alpha = 1.0f;             // Global alpha applies to everything in ImGui
	style.WindowPadding = ImVec2(8, 8);      // Padding within a window
	style.WindowRounding = 0.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);// Alignment for title bar text
	style.FramePadding = ImVec2(4, 1);      // Padding within a framed rectangle (used by most widgets)
	style.FrameRounding = 0.0f;             // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
	style.ItemSpacing = ImVec2(8, 4);      // Horizontal and vertical spacing between widgets/lines
	style.ItemInnerSpacing = ImVec2(4, 4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
	style.TouchExtraPadding = ImVec2(0, 0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
	style.IndentSpacing = 21.0f;            // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
	style.ColumnsMinSpacing = 6.0f;             // Minimum horizontal spacing between two columns
	style.ScrollbarSize = 10.0f;            // Width of the vertical scrollbar, Height of the horizontal scrollbar
	style.ScrollbarRounding = 3.0f;             // Radius of grab corners rounding for scrollbar
	style.GrabMinSize = 10.0f;            // Minimum width/height of a grab box for slider/scrollbar
	style.GrabRounding = 0.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);// Alignment of button text when button is larger than text.
	style.DisplayWindowPadding = ImVec2(22, 22);    // Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
	style.DisplaySafeAreaPadding = ImVec2(4, 4);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
	style.AntiAliasedLines = true;             // Enable anti-aliasing on lines/borders. Disable if you are really short on CPU/GPU.
	style.CurveTessellationTol = 1.25f;            // Tessellation tolerance. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
}

void CMenu::Shutdown()
{
	Globals::PlayerListOpened = false;
	Globals::RadioOpened = false;
	Globals::MenuOpened = false;
	
	SetWindowLongPtrA(FindWindowA(XorStr("Valve001"), nullptr), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_proc));

	ImGui_ImplDX9_Shutdown();
}

LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (Globals::MenuOpened && ImGui_ImplDX9_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	return CallWindowProc(original_proc, hwnd, msg, wParam, lParam);
}

void CMenu::RenderSkinsTab()
{
	static int selected = 0;

	if (ImGui::ButtonSelectable(XorStr("Knife CT"), ImVec2(125, 40), selected == 0, forTabs_text, forTabs_text_x2))
		selected = 0;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Knife T"), ImVec2(125, 40), selected == 1, forTabs_text, forTabs_text_x2))
		selected = 1;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Gloves"), ImVec2(125, 40), selected == 2, forTabs_text, forTabs_text_x2))
		selected = 2;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(ActiveWeaponName.c_str(), ImVec2(125, 40), selected == 3, forTabs_text, forTabs_text_x2))
		selected = 3;

	switch (selected)
	{
	case 0:
		ImGui::Text(XorStr("CT Knife"));

		if (!Vars.weapon[WEAPON_KNIFE].enabled)
			ImGui::Checkbox(XorStr("Disabled"), &Vars.weapon[WEAPON_KNIFE].enabled);
		else
			ImGui::Checkbox(XorStr("Enabled"), &Vars.weapon[WEAPON_KNIFE].enabled);

		if (Vars.weapon[WEAPON_KNIFE].enabled)
		{
			ImGui::InputInt(XorStr("Seed"), &Vars.weapon[WEAPON_KNIFE].seed);
			ImGui::InputInt(XorStr("StatTrak"), &Vars.weapon[WEAPON_KNIFE].stat_trak);
			ImGui::SliderFloat(XorStr("Wear"), &Vars.weapon[WEAPON_KNIFE].wear, 0.f, 1.f, "%.3f");
			ImGui::InputText(XorStr("Name Tag"), Vars.weapon[WEAPON_KNIFE].custom_name, 32);

			ImGui::Combo(XorStr("Paint Kit"), &Vars.weapon[WEAPON_KNIFE].paint_kit_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_skins[idx].name.c_str();
				return true;
			}, nullptr, k_skins.size(), 10);

			ImGui::Combo(XorStr("Knife##Model"), &Vars.weapon[WEAPON_KNIFE].definition_override_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_knife_names.at(idx).name;
				return true;
			}, nullptr, k_knife_names.size(), 5);
		}

		break;
	case 1:
		ImGui::Text(XorStr("T Knife"));

		if (!Vars.weapon[WEAPON_KNIFE_T].enabled)
			ImGui::Checkbox("Disabled", &Vars.weapon[WEAPON_KNIFE_T].enabled);
		else
			ImGui::Checkbox("Enabled", &Vars.weapon[WEAPON_KNIFE_T].enabled);

		if (Vars.weapon[WEAPON_KNIFE_T].enabled)
		{
			ImGui::InputInt("Seed", &Vars.weapon[WEAPON_KNIFE_T].seed);
			ImGui::InputInt("StatTrak", &Vars.weapon[WEAPON_KNIFE_T].stat_trak);
			ImGui::SliderFloat("Wear", &Vars.weapon[WEAPON_KNIFE_T].wear, 0.f, 1.f, "%.3f");
			ImGui::InputText("Name Tag", Vars.weapon[WEAPON_KNIFE_T].custom_name, 32);

			ImGui::Combo("Paint Kit", &Vars.weapon[WEAPON_KNIFE_T].paint_kit_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_skins[idx].name.c_str();
				return true;
			}, nullptr, k_skins.size(), 10);

			ImGui::Combo("Knife##Model", &Vars.weapon[WEAPON_KNIFE_T].definition_override_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_knife_names.at(idx).name;
				return true;
			}, nullptr, k_knife_names.size(), 5);
		}

		break;
	case 2:
		if (!Vars.weapon[5048].enabled)
			ImGui::Checkbox(XorStr("Disabled"), &Vars.weapon[5028].enabled);
		else
			ImGui::Checkbox(XorStr("Enabled"), &Vars.weapon[5028].enabled);

		if (Vars.weapon[5028].enabled)
		{
			ImGui::Combo(XorStr("Glove##model"), &Vars.weapon[5028].definition_override_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_glove_names.at(idx).name;
				return true;
			}, nullptr, k_glove_names.size(), 5);

			ImGui::Combo(XorStr("Paint Kit"), &Vars.weapon[5028].paint_kit_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_gloves[idx].name.c_str();
				return true;
			}, nullptr, k_gloves.size(), 10);
			ImGui::SliderFloat(XorStr("Wear"), &Vars.weapon[5028].wear, 0.f, 1.f, "%.3f");
		}

		break;
	case 3:
		if (!Globals::WeaponTabValid)
			ImGui::Text(XorStr("Take a gun"));
		else {
			if (!WeaponSettings->enabled)
				ImGui::Checkbox(XorStr("Disabled"), &WeaponSettings->enabled);
			else
				ImGui::Checkbox(XorStr("Enabled"), &WeaponSettings->enabled);

			if (WeaponSettings->enabled)
			{
				ImGui::InputInt(XorStr("Seed"), &WeaponSettings->seed);
				ImGui::InputInt(XorStr("StatTrak"), &WeaponSettings->stat_trak);
				ImGui::SliderFloat(XorStr("Wear"), &WeaponSettings->wear, 0.f, 1.f, "%.3f");
				ImGui::InputText(XorStr("Name Tag"), WeaponSettings->custom_name, 32);

				ImGui::Combo(XorStr("Paint Kit"), &WeaponSettings->paint_kit_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_skins[idx].name.c_str();
					return true;
				}, nullptr, k_skins.size(), 10);
			}
		}
		break;
	}

	if (ImGui::Button(XorStr("Update")))
	{
		CSkinChanger::Get().Update();
	}
}

void CMenu::RenderAATab()
{
	static int Selected = 0;

	if (ImGui::ButtonSelectable(XorStr("Standing"), ImVec2(125, 40), Selected == 0, forTabs_text, forTabs_text_x2))
		Selected = 0;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Moving"), ImVec2(125, 40), Selected == 1, forTabs_text, forTabs_text_x2))
		Selected = 1;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Air"), ImVec2(125, 40), Selected == 2, forTabs_text, forTabs_text_x2))
		Selected = 2;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Misc"), ImVec2(125, 40), Selected == 3, forTabs_text, forTabs_text_x2))
		Selected = 3;

	switch (Selected)
	{
	case 0:
	{
		ImGui::Combo("Pitch", &Vars.ragebot_antiaim_pitch, aa_pitch_list, IM_ARRAYSIZE(aa_pitch_list));
		ImGui::Combo("Yaw", &Vars.ragebot_antiaim_yaw, aa_yaw_list, IM_ARRAYSIZE(aa_yaw_list));

		ImGui::Checkbox(XorStr("Desync"), &Vars.ragebot_antiaim_desync);

		if (Vars.ragebot_antiaim_desync)
		{
			ImGui::Combo("Desync Type", &Vars.ragebot_antiaim_desynctype, desynctype, IM_ARRAYSIZE(desynctype));
		}

		break;
	}

	case 1:
	{
		ImGui::Combo("Pitch", &Vars.ragebot_antiaim_pitch_move, aa_pitch_list, IM_ARRAYSIZE(aa_pitch_list));
		ImGui::Combo("Yaw", &Vars.ragebot_antiaim_yaw_move, aa_yaw_list, IM_ARRAYSIZE(aa_yaw_list));

		ImGui::Checkbox(XorStr("Desync"), &Vars.ragebot_antiaim_desync);

		if (Vars.ragebot_antiaim_desync)
		{
			ImGui::Combo("Desync Type", &Vars.ragebot_antiaim_desynctype, desynctype, IM_ARRAYSIZE(desynctype));
		}

		break;
	}

	case 2:
	{
		ImGui::Combo("Pitch", &Vars.ragebot_antiaim_pitch_air, aa_pitch_list, IM_ARRAYSIZE(aa_pitch_list));
		ImGui::Combo("Yaw", &Vars.ragebot_antiaim_yaw_air, aa_yaw_list_air, IM_ARRAYSIZE(aa_yaw_list_air));

		break;
	}

	case 3:
	{
		ImGui::SliderFloat("Spin Speed", &Vars.ragebot_antiaim_spinspeed, -20, 20, ("%.0f"));

		ImGui::Spacing();

		ImGui::Combo(XorStr("Fakelag Type"), &Vars.ragebot_fakelag_type, std::vector<std::string>{ "Off", "Factor" });

		ImGui::SliderFloat("Fakelag Ticks", &Vars.ragebot_fakelag_amt, 1, 14, ("%.1f"));

		static std::string prevValue2 = "Select";

		if (ImGui::BeginCombo("Fakelag When", "Select", 0))
		{
			std::vector<std::string> vec2;

			for (size_t i = 0; i < IM_ARRAYSIZE(type); i++)
			{
				ImGui::Selectable(type[i], &Vars.ragebot_fakelag_flags[i], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);

				if (Vars.ragebot_fakelag_flags[i])
					vec2.push_back(type[i]);
			}

			for (size_t i = 0; i < vec2.size(); i++)
			{
				if (vec2.size() == 1)
					prevValue2 += vec2.at(i);
				else if (i != vec2.size())
					prevValue2 += vec2.at(i) + ", ";
				else
					prevValue2 += vec2.at(i);
			}

			ImGui::EndCombo();
		}

		break;
	}
	}
}

void CMenu::RenderRagebotTab()
{
	static int selected = 0;

	if (ImGui::ButtonSelectable(XorStr("Globals"), ImVec2(270, 40), selected == 0, forTabs_text, forTabs_text_x2))
		selected = 0;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Weapon"), ImVec2(270, 40), selected == 1, forTabs_text, forTabs_text_x2))
		selected = 1;

	if (selected == 0) 
	{
		ImGui::BeginGroup();
		{
			ImGui::PushItemWidth(152);

			if (!Vars.ragebot_enabled)
				ImGui::Checkbox(XorStr("Ragebot Disabled"), &Vars.ragebot_enabled);
			else
				ImGui::Checkbox(XorStr("Ragebot Enabled"), &Vars.ragebot_enabled);

			if (Vars.ragebot_enabled)
			{
				ImGui::Checkbox(XorStr("Auto zeus"), &Vars.ragebot_autozeus);

				ImGui::Text(XorStr("FOV"));
				ImGui::SliderInt(XorStr("##FOV"), &Vars.ragebot_fov, 0, 180);
				ImGui::PopItemWidth();

				ImGui::PushItemWidth(152);

				ImGui::SliderInt("Slowwalk", &Vars.ragebot_slowwalk_amt, 0, 100);
				ImGui::Combo(XorStr("Slowwalk key"), &Vars.ragebot_slowwalk_key, AllKeys, IM_ARRAYSIZE(AllKeys));

				ImGui::Spacing();

				ImGui::Checkbox(XorStr("Resolver"), &Vars.ragebot_resolver);

				ImGui::PopItemWidth();
			}
		}
		ImGui::EndGroup();

	}
	else if (selected == 1) 
	{
		static int curGroup = 0;
		if (ImGui::ButtonSelectable("Pistols", ImVec2(75, 25), curGroup == WEAPON_GROUPS::PISTOLS)) curGroup = WEAPON_GROUPS::PISTOLS;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("Rifles", ImVec2(75, 25), curGroup == WEAPON_GROUPS::RIFLES)) curGroup = WEAPON_GROUPS::RIFLES;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("SMG", ImVec2(75, 25), curGroup == WEAPON_GROUPS::SMG)) curGroup = WEAPON_GROUPS::SMG;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("Shotguns", ImVec2(75, 25), curGroup == WEAPON_GROUPS::SHOTGUNS)) curGroup = WEAPON_GROUPS::SHOTGUNS;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("Scout", ImVec2(75, 25), curGroup == WEAPON_GROUPS::SCOUT)) curGroup = WEAPON_GROUPS::SCOUT;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("Auto", ImVec2(75, 25), curGroup == WEAPON_GROUPS::AUTO)) curGroup = WEAPON_GROUPS::AUTO;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("AWP", ImVec2(75, 25), curGroup == WEAPON_GROUPS::AWP)) curGroup = WEAPON_GROUPS::AWP;
		ImGui::BeginGroup();
		{
			ImGui::PushItemWidth(142);

			ImGui::Checkbox(XorStr("Auto Scope"), &Vars.ragebot_autoscope[curGroup]);

			ImGui::SliderFloat(XorStr("Hitchance"), &Vars.ragebot_hitchance[curGroup], 0.f, 100.f);
			ImGui::SliderFloat(XorStr("Min Damage"), &Vars.ragebot_mindamage[curGroup], 0.f, 100.f);

			ImGui::Combo("Target Selection", &Vars.ragebot_selection[curGroup], selectiontypes, IM_ARRAYSIZE(selectiontypes));

			static std::string prevValue = "Select";
			if (ImGui::BeginCombo("Hitscan", "Select", 0))
			{
				std::vector<std::string> vec;
				for (size_t i = 0; i < IM_ARRAYSIZE(hitboxes); i++)
				{
					ImGui::Selectable(hitboxes[i], &Vars.ragebot_hitbox[i][curGroup], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
					if (Vars.ragebot_hitbox[i][curGroup])
						vec.push_back(hitboxes[i]);
				}

				for (size_t i = 0; i < vec.size(); i++)
				{
					if (vec.size() == 1)
						prevValue += vec.at(i);
					else if (i != vec.size())
						prevValue += vec.at(i) + ", ";
					else
						prevValue += vec.at(i);
				}
				ImGui::EndCombo();
			} 
			ImGui::PopItemWidth();
		}
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		{
			ImGui::PushItemWidth(142);
			ImGui::Text("Multipoint Scale");
			for (int i = 0; i < 8; i++)
			{
				if (Vars.ragebot_hitbox[i])
					ImGui::SliderFloat(hitboxes[i], &Vars.ragebot_hitbox_multipoint_scale[i][curGroup], 0.f, 1.f);
			}
			ImGui::PopItemWidth();
		}
		ImGui::EndGroup();
	}
}

void CMenu::RenderVisualsTab()
{
	static int selected = 0;

	if (ImGui::ButtonSelectable(XorStr("Players"), ImVec2(125, 40), selected == 0, forTabs_text, forTabs_text_x2))
		selected = 0;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Misc##Visuals_tab"), ImVec2(125, 40), selected == 1, forTabs_text, forTabs_text_x2))
		selected = 1;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Colors##Visuals_tab"), ImVec2(125, 40), selected == 2, forTabs_text, forTabs_text_x2))
		selected = 2;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Viewmodel"), ImVec2(125, 40), selected == 3, forTabs_text, forTabs_text_x2))
		selected = 3;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Item ESP"), ImVec2(125, 40), selected == 4, forTabs_text, forTabs_text_x2))
		selected = 4;

	switch (selected)
	{
	case 0:
		ImGui::BeginGroup();

		if (!Vars.esp_enabled)
			ImGui::Checkbox(XorStr("ESP Disabled"), &Vars.esp_enabled);
		else
			ImGui::Checkbox(XorStr("ESP Enabled"), &Vars.esp_enabled);

		if (Vars.esp_enabled)
		{
			ImGui::Checkbox(XorStr("Ignore team"), &Vars.esp_ignore_team);
			ImGui::Checkbox(XorStr("Visible only"), &Vars.esp_visible_only);

			ImGui::Spacing();

			ImGui::Checkbox(XorStr("Box"), &Vars.esp_player_boxes);
			ImGui::Checkbox(XorStr("Name"), &Vars.esp_player_names);
			ImGui::Checkbox(XorStr("Health"), &Vars.esp_player_health);
			ImGui::Checkbox(XorStr("Armor"), &Vars.esp_player_armour);
			ImGui::Checkbox(XorStr("Weapon"), &Vars.esp_player_weapons);

			ImGui::Spacing();

			ImGui::Checkbox(XorStr("Enable chams##CHAMS"), &Vars.chams_player_enabled);
			ImGui::Checkbox(XorStr("Ignore team##CHAMS"), &Vars.chams_player_ignore_team);
			ImGui::Checkbox(XorStr("Visible only##CHAMS"), &Vars.chams_player_visible_only);

			ImGui::Spacing();

			ImGui::Checkbox(XorStr("Snaplines"), &Vars.visuals_snapline);
			if (Vars.visuals_snapline)
				ImGui::ColorEdit4(XorStr("Snapline Colour"), Vars.visuals_snaplines, ImGuiColorEditFlags_NoInputs);

			if (Vars.esp_dropped_weapons)
				ImGui::ColorEdit4(XorStr("Dropped Weapons Colour"), Vars.esp_dropped_clr, ImGuiColorEditFlags_NoInputs);

			if (Vars.esp_planted_c4)
				ImGui::ColorEdit4(XorStr("Planted C4 Colour"), Vars.esp_planted_c4_clr, ImGuiColorEditFlags_NoInputs);
		}

		ImGui::EndGroup();
		break;
	case 1:
		ImGui::BeginGroup();

		ImGui::Text(XorStr("Misc visuals"));

		ImGui::Checkbox(XorStr("Radar"), &Vars.visuals_radar);

		ImGui::Spacing();

		ImGui::Checkbox(XorStr("No visual recoil"), &Vars.visuals_norecoil);
		ImGui::Checkbox(XorStr("No flash"), &Vars.visuals_noflash);
		ImGui::Checkbox(XorStr("No Smoke"), &Vars.visuals_nosmoke);
		ImGui::Checkbox(XorStr("No Scope Zoom"), &Vars.visuals_noscopezoom);
		ImGui::Checkbox(XorStr("Remove scope borders"), &Vars.visuals_noscopeborder);

		ImGui::Checkbox(XorStr("Full Bright"), &Vars.visuals_fullbright);
		ImGui::Checkbox(XorStr("Ind"), &Vars.visuals_ind);

		ImGui::Checkbox(XorStr("Thirdperson"), &Vars.visuals_thirdperson); ImGui::SameLine();
		ImGui::PushItemWidth(60);
		ImGui::Combo(XorStr("##key"), &Vars.visuals_thirdperson_key, AllKeys, IM_ARRAYSIZE(AllKeys));

		ImGui::PopItemWidth();

		ImGui::Checkbox(XorStr("Watermark"), &Vars.visuals_watermark);

		if (Vars.visuals_watermark == true)
			ImGui::ColorEdit4(XorStr("Filled Colour"), Vars.color_counter, ImGuiColorEditFlags_NoInputs);

		ImGui::EndGroup();
		break;
	case 2:
		ImGui::BeginGroup();
		ImGui::Text(XorStr("Esp"));
		ImGui::ColorEdit4(XorStr("Enemy visible##esp"), Vars.color_esp_enemy_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Enemy hidden##esp"), Vars.color_esp_enemy_hidden, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team visible##esp"), Vars.color_esp_team_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team hidden##esp"), Vars.color_esp_team_hidden, ImGuiColorEditFlags_NoInputs);

		ImGui::Text(XorStr("Chams"));
		ImGui::ColorEdit4(XorStr("Enemy visible##chams"), Vars.color_chams_enemy_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Enemy hidden##chams"), Vars.color_chams_enemy_hidden, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team visible##chams"), Vars.color_chams_team_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team hidden##chams"), Vars.color_chams_team_hidden, ImGuiColorEditFlags_NoInputs);

		ImGui::Text(XorStr("Menu"));
		ImGui::ColorEdit4(XorStr("HUD Colour"), Vars.color_menu_shit, ImGuiColorEditFlags_NoInputs);
		ImGui::EndGroup();
		break;
	case 3:
	{
		ImGui::Text(XorStr("Viewmodel"));

		ImGui::SliderInt(XorStr("Override Fov"), &Vars.misc_overridefov, 0, 180);

		ImGui::Checkbox("Force In Scope", &Vars.misc_overridefov_inscope);

		ImGui::SliderInt(XorStr("Viewmodel Fov"), &Vars.misc_viewmodelfov, 0, 180);

		break;
	}
	case 4:
	{
		ImGui::Text(XorStr("Item ESP"));

		if (!Vars.esp_drop_enable)
			ImGui::Checkbox("Disabled", &Vars.esp_drop_enable);
		else
			ImGui::Checkbox("Enabled", &Vars.esp_drop_enable);

		if (Vars.esp_drop_enable)
		{
			ImGui::Text(XorStr("Distance"));
			ImGui::SliderInt(XorStr("##Distance"), &Vars.esp_drop_distance, 0, 3500);

			ImGui::Spacing();

			ImGui::Checkbox(XorStr("Dropped Weapons"), &Vars.esp_dropped_weapons);
			ImGui::Checkbox(XorStr("Planted C4"), &Vars.esp_planted_c4);

			ImGui::Spacing();

			ImGui::Checkbox(XorStr("Pistol Case"), &Vars.esp_case_pistol);
			ImGui::Checkbox(XorStr("Light Case"), &Vars.esp_case_light_weapon);
			ImGui::Checkbox(XorStr("Heavy Case"), &Vars.esp_case_heavy_weapon);
			ImGui::Checkbox(XorStr("Explosive Case"), &Vars.esp_case_explosive);
			ImGui::Checkbox(XorStr("Tools case"), &Vars.esp_case_tools);
			ImGui::Checkbox(XorStr("Airdrop"), &Vars.esp_random);
			ImGui::Checkbox(XorStr("Full Armor"), &Vars.esp_dz_armor_helmet);
			ImGui::Checkbox(XorStr("Helmet"), &Vars.esp_dz_helmet);
			ImGui::Checkbox(XorStr("Armor"), &Vars.esp_dz_armor);
			ImGui::Checkbox(XorStr("Tablet Upgrade"), &Vars.esp_upgrade_tablet);
			ImGui::Checkbox(XorStr("Briefcase"), &Vars.esp_briefcase);
			ImGui::Checkbox(XorStr("Parachute"), &Vars.esp_parachutepack);
			ImGui::Checkbox(XorStr("Cash Dufflebag"), &Vars.esp_dufflebag);
			ImGui::Checkbox(XorStr("Ammobox"), &Vars.esp_ammobox);
		}

		break;
	}
	}
}

void CMenu::RenderMiscTab()
{
	ImGui::PushItemWidth(150);

	ImGui::Checkbox(XorStr("BunnyHop"), &Vars.misc_bhop);
	ImGui::Checkbox(XorStr("FastDuck"), &Vars.misc_fastduck);
	ImGui::Checkbox(XorStr("AutoStrafe"), &Vars.misc_autostrafe);
	ImGui::Checkbox(XorStr("Fake Duck"), &Vars.misc_fakeduck);

	ImGui::SameLine();

	ImGui::Combo(XorStr("Key##"), &Vars.misc_fakeduck_key, AllKeys, IM_ARRAYSIZE(AllKeys));

	ImGui::Checkbox(XorStr("AutoAccept"), &Vars.misc_autoaccept);

	ImGui::Checkbox(XorStr("Spectator List"), &Vars.misc_spectlist);
}

void CMenu::RenderInfoTab()
{

}

void CMenu::MainFrame()
{
	if (!Globals::MenuOpened)
		return;

	static int tab;

	int screen_width, screen_height;
	Interfaces::Engine->GetScreenSize(screen_width, screen_height);

	ImGui::Begin("##info", &Globals::MenuOpened, ImVec2(200, 500), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	{
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 2), Vars.barColor1.x, Vars.barColor1.y, Vars.barColor1.h, Vars.barColor1.w);
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x - 5, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y + 2), Vars.barColor2.x, Vars.barColor2.y, Vars.barColor2.h, Vars.barColor2.w);

		ImGui::Spacing();

		ImGui::Text("ChangeLogs:    \n");

		ImGui::Text("   20/03/19: \n     - Fixed Most Crashes\n");
		ImGui::Text("   19/03/19: \n     - Improved Desync\n     - Improved Resolver\n     - Improved Fake Duck\n     - Changed Health & Armor Visuals\n");
		ImGui::Text("   18/03/19: \n     - Improved Desync\n     - Fixed Visual Glitch\n");
		ImGui::Text("   17/03/19: \n     - Recoded Antiaim\n     - Improved Desync\n");
		ImGui::Text("   16/03/19: \n     - Improved Desync\n");
		ImGui::Text("   15/03/19: \n     - Added Better Fake Crouch\n     - Added No Scope Zoom\n");
		ImGui::Text("   14/03/19: \n     - Added A Desync Resolver\n     - Improved Desync Resolver\n");
		ImGui::Text("   13/03/19: \n     - Desync Working As It Should\n");
		ImGui::Text("   12/03/19: \n     - Added Some Visual Options\n");
		ImGui::Text("   11/03/19: \n     - Desync Changes\n");
		ImGui::Text("   10/03/19: \n     - Menu Fixes\n     - Desync Changes\n");
		ImGui::Text("   09/03/19: \n     - Changes To The Menu\n");
		ImGui::Text("   08/03/19: \n     - Fixed Desync Crash\n");
		ImGui::Text("   07/03/19: \n     - Fixed Crashing On Warmup End\n     - Fixed Antiaim\n     - Added Better Desync\n     - Added New Anitaim");

		ImGui::Text("If you have any problems with\nThe cheat ask the owner\nDiscord: vizecs#8835\nWebsite: Working On It");
	}
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(800, 600));
	if (ImGui::BeginAndroid(XorStr("Sketu.xyz"), &Globals::MenuOpened, true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 2), Vars.barColor1.x, Vars.barColor1.y, Vars.barColor1.h, Vars.barColor1.w);
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x - 5, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y + 2), Vars.barColor2.x, Vars.barColor2.y, Vars.barColor2.h, Vars.barColor2.w);

		// 43

		ImGui::BeginChild("##tabs", ImVec2(100, 557));
		{
			ImGui::SameLine(18);
			ImGui::Dummy(ImVec2(0, 70));

			ImGui::Dummy(ImVec2(0, 0));

			ImGui::PushFont(forTabs);

			if (ImGui::Button(XorStr("A"), ImVec2(100, 80)))
				tabselected = 0;
			if (ImGui::Button(XorStr("C"), ImVec2(100, 80)))
				tabselected = 1;
			if (ImGui::Button(XorStr("D"), ImVec2(100, 80)))
				tabselected = 2;
			if (ImGui::Button(XorStr("I"), ImVec2(100, 80)))
				tabselected = 3;
			if (ImGui::Button(XorStr("G"), ImVec2(100, 80)))
				tabselected = 4;

			ImGui::PopFont();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginGroup();
		{
			switch (tabselected)
			{
			case 0:
				RenderRagebotTab();
				break;
			case 1:
				RenderAATab();
				break;
			case 2:
				RenderVisualsTab();
				break;
			case 3:
				RenderSkinsTab();
				break;
			case 4:
				RenderMiscTab();
				break;
			}
		}
		ImGui::EndGroup();

		ImGui::BeginGroup();

		ImGui::PushItemWidth(140);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.045f, 0.045f, 0.045f, 1.00f));

		ImGui::Combo(XorStr("##configselect"), &a1, items, ARRAYSIZE(items));
		ImGui::PopStyleColor();

		ImGui::PopItemWidth();

		ImGui::SameLine();

		if (ImGui::ButtonGradientEx(XorStr("Load"), ImVec2(140, 24)))
		{
			CConfig::Get().Load(items[a1]);
			CSkinChanger::Get().Update();
		}
		ImGui::SameLine();
		if (ImGui::ButtonGradientEx(XorStr("Save"), ImVec2(140, 24)))
		{
			CConfig::Get().Save(items[a1]);
		}
		ImGui::SameLine();
		if (ImGui::ButtonGradientEx(XorStr("Unload"), ImVec2(140, 24)))
		{
			Globals::Unload = true;
		}

		ImGui::EndGroup();
		ImGui::End();
	}
}

bool CMenu::Initialize(IDirect3DDevice9* device)
{
	static bool once = false;
	if (!once)
	{
		HWND hWindow = FindWindowA(XorStr("Valve001"), 0);

		ImGui_ImplDX9_Init(hWindow, device);
		ImGuiStyle& style = ImGui::GetStyle();

		ImStyles(ImGui::GetStyle());
		ImColors(ImGui::GetStyle());

		ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Ubuntu_compressed_data, Ubuntu_compressed_size, 13, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
		forTabs = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\cherryfont.ttf", 40, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
		forTabs_text = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Ubuntu_compressed_data, Ubuntu_compressed_size, 35, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
		forTabs_text_x2 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Ubuntu_compressed_data, Ubuntu_compressed_size, 18, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
		forTabs_text_x22 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Ubuntu_compressed_data, Ubuntu_compressed_size, 16, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

		ImGui_ImplDX9_CreateDeviceObjects();
		original_proc = (WNDPROC)SetWindowLongA(hWindow, GWL_WNDPROC, (LONG)(LONG_PTR)proc);

		ImFontConfig config;
		config.FontDataOwnedByAtlas = false;

		ImGui::GetIO().IniFilename = nullptr;
		once = true;
	}
	return once;
}

void CMenu::SpectatorListFrame()
{
	if (!Vars.misc_spectlist)
		return;

	int specs = 0;
	int modes = 0;
	std::string spect = "";
	std::string mode = "";
	int DrawIndex = 1;

	for (int playerId : Utils::GetObservervators(Interfaces::Engine->GetLocalPlayer()))
	{
		if (playerId == Interfaces::Engine->GetLocalPlayer())
			continue;

		C_BasePlayer* pPlayer = (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(playerId);

		if (!pPlayer)
			continue;

		player_info_t Pinfo;
		Interfaces::Engine->GetPlayerInfo(playerId, &Pinfo);

		if (Pinfo.fakeplayer)
			continue;

		spect += Pinfo.szName;
		spect += "\n";
		specs++;
	}
	bool misc_Spectators = true;

	if (ImGui::BeginAndroid(XorStr("Spectator List - Sketu.xyz"), &misc_Spectators, false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x + 100, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 2), Vars.barColor1.x, Vars.barColor1.y, Vars.barColor1.h, Vars.barColor1.w);
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x - 5, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y + 2), Vars.barColor2.x, Vars.barColor2.y, Vars.barColor2.h, Vars.barColor2.w);
		if (specs > 0)
			spect += "\n";
		ImVec2 size = ImGui::CalcTextSize(spect.c_str());

		ImGui::SetWindowSize(ImVec2(200, 25 + size.y));

		ImGui::Text(spect.c_str());
		ImGui::NextColumn();
		DrawIndex++;
	}
	ImGui::End();
}

void CMenu::EndScene(IDirect3DDevice9* device)
{	
	Globals::WeaponTabValid = Utils::IsInGame() && Globals::LocalPlayer->IsAlive() && Globals::LocalPlayer->m_hActiveWeapon()->IsGun();

	if (CMenu::Initialize(device))
	{
		if (Utils::IsKeyPressed(VK_INSERT))
			Globals::MenuOpened = !Globals::MenuOpened;

		ImColor clr =
			ImColor(Vars.barcolour[0],
				Vars.barcolour[1],
				Vars.barcolour[2],
				Vars.barcolour[3]
			);

		if (isSkeet)
		{
			Vars.barColor1 = CVariables::Colour4(ImColor(201, 84, 192), ImColor(204, 227, 54), ImColor(204, 227, 54), ImColor(201, 84, 192));
			Vars.barColor2 = CVariables::Colour4(ImColor(55, 177, 218), ImColor(201, 84, 192), ImColor(201, 84, 192), ImColor(55, 177, 218));
		}
		else 
		{
			Vars.barColor1 = CVariables::Colour4(ImColor(201, 84, 192), ImColor(204, 227, 54), ImColor(204, 227, 54), ImColor(201, 84, 192));
			Vars.barColor2 = CVariables::Colour4(ImColor(55, 177, 218), ImColor(201, 84, 192), ImColor(201, 84, 192), ImColor(55, 177, 218));
		}

		DWORD dwOld_D3DRS_COLORWRITEENABLE; IDirect3DVertexDeclaration9* vertDec; IDirect3DVertexShader9* vertShader;
		device->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		device->GetVertexDeclaration(&vertDec);
		device->GetVertexShader(&vertShader);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);
		ImGui_ImplDX9_NewFrame();

		CMenu::MainFrame();
		CMenu::SpectatorListFrame();

		ImGui::Render();
		device->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		device->SetVertexDeclaration(vertDec);
		device->SetVertexShader(vertShader);
	}
} 

void CMenu::InvalidateDeviceObjects() { ImGui_ImplDX9_InvalidateDeviceObjects(); }
void CMenu::CreateDeviceObjects() { ImGui_ImplDX9_CreateDeviceObjects(); }