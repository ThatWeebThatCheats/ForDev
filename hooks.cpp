#include "hooks.hpp"
#include <intrin.h>  
#include <time.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "Valve_SDK/interfaces/Logger.h"
#include "Menu/menu.hpp"
#include "options.hpp"
#include "Helpers/input.hpp"
#include "Helpers/utils.hpp"
#include "Features/misc/misc.hpp"
#include "Features/visuals/visuals.hpp"
#include "Features/backtrack/backtrack.hpp"
#include "Features/chams/chams.hpp"
#include "Features/skinchanger/skins.hpp"
#include "Features/ragebot/ragebot.hpp"
#include "Features/ragebot/antiaim.hpp"
#include "Features/engineprediction/engineprediction.hpp"
#include "Features/backtrack/timewarp.hpp"
#include "Features/visuals/hitmarkers.h"
#include "Features/visuals/nightmode.hpp"
#include "parser.h"

#pragma intrinsic(_ReturnAddress) 

namespace Hooks
{
	vfunc_hook hlclient_hook;
	vfunc_hook event_hook;
	vfunc_hook direct3d_hook;
	vfunc_hook vguipanel_hook;
	vfunc_hook vguisurf_hook;
	vfunc_hook sound_hook;
	vfunc_hook mdlrender_hook;
	vfunc_hook renderview_hook;
	vfunc_hook clientmode_hook;
	vfunc_hook sv_cheats;
	vfunc_hook netgraphtext_hook;
	vfunc_hook net_channel_hook_manager;
	vfunc_hook FindOrAddFileName_hook;
	SendDatagram_t oSendDatagram = NULL;
	static bool dgram_hooked = false;

	using Hooked_FindOrAddFileNamefn = void*(__thiscall*)(IFileSystem*, char const*);

	void Initialize()
	{
		hlclient_hook.setup(Interfaces::Client);
		event_hook.setup(Interfaces::GameEvents);
		direct3d_hook.setup(Interfaces::D3DDevice);
		vguipanel_hook.setup(Interfaces::Panel);
		vguisurf_hook.setup(Interfaces::Surface);
		sound_hook.setup(Interfaces::EngineSound);
		mdlrender_hook.setup(Interfaces::ModelRender);
		renderview_hook.setup(Interfaces::RenderView);
		clientmode_hook.setup(Interfaces::ClientMode);
		FindOrAddFileName_hook.setup(Interfaces::FileSystem);
		sv_cheats.setup(Interfaces::Convar->FindVar(XorStr("sv_cheats")));

		CChams::Get().Initialize();
		CSkinChanger::Get().HookSequence();

		Interfaces::Engine->GetScreenSize(Globals::ScreenWeight, Globals::ScreenHeight);

		direct3d_hook.hook_index(index::EndScene, hookEndScene);
		direct3d_hook.hook_index(index::Reset, hookReset);
		
		renderview_hook.hook_index(index::SceneEnd, hookSceneEnd);
		hlclient_hook.hook_index(index::FrameStageNotify, hookFrameStageNotify);
		clientmode_hook.hook_index(index::CreateMove, hookCreateMove);
		clientmode_hook.hook_index(index::OverrideView, hookOverrideView);
		clientmode_hook.hook_index(index::ViewModel, hookViewModelFov);
		event_hook.hook_index(index::FireEventClientSide, hookFireEventClientSideThink);		
		vguipanel_hook.hook_index(index::PaintTraverse, hookPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hookEmitSound);
		vguisurf_hook.hook_index(index::LockCursor, hookLockCursor);
		sv_cheats.hook_index(index::SvCheatsGetBool, hookSvCheatsGetBool);
	}

	void Unload()
	{
		Utils::DetachConsole();

		hardDisable = true;
		CChams::Get().Shutdown();
		CMenu::Get().Shutdown();
		CHudModulation::Get().ToggleVGuiModulation(false);

		hlclient_hook.unhook_all();
		net_channel_hook_manager.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		renderview_hook.unhook_all();
		clientmode_hook.unhook_all();
		netgraphtext_hook.unhook_all();
	}

	bool __fastcall hookNetGraph(void* ecx, void* edx)
	{
		static auto ofunc = netgraphtext_hook.get_original<netgraphtextFn>(index::NetGraph);

		void* returna = _ReturnAddress();
		static auto v1 = (DWORD)Utils::PatternScan(GetModuleHandleA(XorStr("client_panorama.dll")), XorStr("85 C0 0F 84 ? ? ? ? A1 ? ? ? ? 0F 57 C0 F3 0F 10 48"));
		static auto cnetgraph = **(uintptr_t**)(Utils::PatternScan(GetModuleHandleA(XorStr("client_panorama.dll")), XorStr("89 1D ? ? ? ? 8B C3 5B 8B E5 5D C2 04")) + 2);

		return ofunc(ecx);
	}

	bool __fastcall hookDispatchUserMessage(void* ecx, void* edx, int type, unsigned int a3, unsigned int length, const void *msg_data)
	{
		static auto oDispatchUserMessage = hlclient_hook.get_original<tDispatchUserMessage>(index::DispatchUserMessage);

		// CMisc::Get().AntiKick(oDispatchUserMessage, ecx, type, a3, length, msg_data);

		return oDispatchUserMessage(ecx, type, a3, length, msg_data);
	}

	long __stdcall hookEndScene(IDirect3DDevice9* pDevice)
	{
		auto oEndScene = direct3d_hook.get_original<EndScene>(index::EndScene);

		CMenu::Get().EndScene(pDevice);

		return oEndScene(pDevice);
	}

	void __stdcall hookLockCursor()
	{
		static auto ofunc = vguisurf_hook.get_original<LockCursor_t>(index::LockCursor);

		if (Globals::MenuOpened) {
			Interfaces::Surface->UnlockCursor();
			return;
		}
		ofunc(Interfaces::Surface);
	}

	void __fastcall hookSceneEnd(void* thisptr, void* edx)
	{
		static auto oSceneEnd = renderview_hook.get_original<iSceneEnd>(index::SceneEnd);
		oSceneEnd(thisptr, edx);

		CChams::Get().SceneEnd();
	}

	long __stdcall hookReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		auto oReset = direct3d_hook.get_original<Reset>(index::Reset);

		CMenu::Get().InvalidateDeviceObjects();

		auto hr = oReset(device, pPresentationParameters);

		if (SUCCEEDED(hr))
		{
			Interfaces::Engine->GetScreenSize(Globals::ScreenWeight, Globals::ScreenHeight);
			CDraw::Get().SetupFonts();
			CMenu::Get().CreateDeviceObjects();
		}

		return hr;
	}

	//SendDatagram_t original_dgram;
	int __fastcall hookSendDatagram(void * net_channel, void *, void * datagram)
	{
		if (!Vars.misc_latency_enable || !Utils::IsInGame())
			return oSendDatagram(net_channel, datagram);

		auto * channel = reinterpret_cast<INetChannel*>(net_channel);

		int instate = channel->m_nInReliableState;
		int insequencenr = channel->m_nInSequenceNr;

		CBacktrack::Get().AddLatency(channel, Vars.misc_latency_amt / 1000.f);

		int ret = oSendDatagram(channel, datagram);

		channel->m_nInReliableState = instate;
		channel->m_nInSequenceNr = insequencenr;

		return ret;
	}

	bool __stdcall hookCreateMove(float smt, CUserCmd *cmd)
	{
		uintptr_t* fp;
		__asm mov fp, ebp;
		bool* bSendPacket = (bool*)(*fp - 0x1C);

		static auto original = clientmode_hook.get_original<CreateMove_t>(24);

		if (!cmd->command_number || !Interfaces::Engine->IsInGame() || !Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive())
			return original(Interfaces::ClientMode, smt, cmd);

		CRageBot::Get().PrecacheShit();

		CMisc::Get().Bhop(cmd);
		CMisc::Get().AutoStrafe(cmd);
		CMisc::Get().FastDuck(cmd);

		CPredictionSystem::Get().Start(cmd, Globals::LocalPlayer);
		{		
			CMisc::Get().FakeDuck(cmd, *bSendPacket);
			CMisc::Get().SlowWalk(cmd);
			CMisc::Get().FakeLag(cmd, *bSendPacket);
			CRageBot::Get().CreateMove(cmd, *bSendPacket);
			CAntiAim::Get().CreateMove(cmd, *bSendPacket);
		}
		CPredictionSystem::Get().End(Globals::LocalPlayer);

		if (Globals::LocalPlayer->m_hActiveWeapon()->IsGun()) 
		{
			WeaponSettings = &Vars.weapon[Utils::GetWeaponSettingsSelectID()];
			ActiveWeaponName = Utils::pWeaponData[Utils::GetWeaponSettingsSelectID()].c_str();
		}
		else 
		{
			WeaponSettings = &Vars.weapon[0];
			ActiveWeaponName = XorStr("Weapon");
		}

		Math::NormalizeAngles(cmd->viewangles);
		Math::ClampAngles(cmd->viewangles);
		Globals::LastAngle = cmd->viewangles;
		QAngle oldAngle;
		float oldForward;
		float oldSideMove;
		Interfaces::Engine->GetViewAngles(oldAngle);
		oldForward = cmd->forwardmove;
		oldSideMove = cmd->sidemove;

		if (Globals::LocalPlayer->m_nMoveType() != MOVETYPE_LADDER)
			Utils::CorrectMovement(oldAngle, cmd, oldForward, oldSideMove);
		return false;
	}

	void __stdcall hookPaintTraverse(vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<PaintTraverse>(index::PaintTraverse);

		const auto panelName = Interfaces::Panel->GetName(panel);
		bool gameUIBeingRendered = false;

		if (Globals::LocalPlayer && Globals::LocalPlayer->IsAlive() && Vars.visuals_noscopeborder && !strcmp(XorStr("HudZoom"), panelName))
			return;

		if (!strcmp(XorStr("Fullscreen Root Panel"), panelName) || !strcmp(XorStr("CounterStrike Root Panel"), panelName))
			gameUIBeingRendered = true;

		if (gameUIBeingRendered)
			CHudModulation::Get().ToggleVGuiModulation(false);

		oPaintTraverse(Interfaces::Panel, panel, forceRepaint, allowForce);

		if (gameUIBeingRendered)
			CHudModulation::Get().ToggleVGuiModulation(true);

		CMisc::Get().ClanTag();

		static unsigned int FocusOverlayPanel = 0;
		static bool FoundPanel = false;

		if (!panelId)
		{
			if (!strcmp(panelName, XorStr("FocusOverlayPanel"))) 
			{
				CDraw::Get().SetupFonts();
				panelId = panel;
			}
		}
		if (panelId == panel)
		{
			CHudModulation::Get().ToggleVGuiModulation(false);
			Interfaces::Surface->PushMakeCurrent(panel, false);
			CVisuals::Get().PaintTraverse();
			Interfaces::Surface->PopMakeCurrent(panel);
			CHudModulation::Get().ToggleVGuiModulation(true);
		}
	}

	void __stdcall hookEmitSound(IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk)
	{
		static auto ofunc = sound_hook.get_original<EmitSound1>(index::EmitSound1);

		CMisc::Get().AutoAccept(pSoundEntry);

		ofunc(Interfaces::EngineSound, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);
	}

	void __stdcall hookFrameStageNotify(ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<FrameStageNotify>(index::FrameStageNotify);
		CHudModulation::Get().FrameStageNotify(stage);
		CSkinChanger::Get().OnFrameStageNotify(stage);
		CRageBot::Get().PVSFix(stage);
		CMisc::Get().SetThirdpersonAngles(stage);

		QAngle aim_punch_old;
		QAngle view_punch_old;
		QAngle* aim_punch = nullptr;
		QAngle* view_punch = nullptr;

		if (Vars.visuals_norecoil && stage == ClientFrameStage_t::FRAME_RENDER_START)
		{
			if (Globals::LocalPlayer && Globals::LocalPlayer->IsAlive())
			{
				aim_punch = &Globals::LocalPlayer->m_aimPunchAngle();
				view_punch = &Globals::LocalPlayer->m_viewPunchAngle();

				aim_punch_old = *aim_punch;
				view_punch_old = *view_punch;

				*aim_punch = QAngle(0, 0, 0);
				*view_punch = QAngle(0, 0, 0);
			}
		}

		CFullBright::Get().Apply();
		CVisuals::Get().NoSmoke();

		if (Globals::LocalPlayer->IsAlive())
			Globals::LocalPlayer->m_flFlashMaxAlpha() = Vars.visuals_noflash ? 0.f : 255.f;

		ofunc(Interfaces::Client, stage);
		if (aim_punch && view_punch)
		{
			*aim_punch = aim_punch_old;
			*view_punch = view_punch_old;
		}
	}

	bool __fastcall hookFireEventClientSideThink(void* ecx, void* edx, IGameEvent* pEvent)
	{
		static auto oFireEventClientSide = event_hook.get_original<FireEventClientSide>(index::FireEventClientSide);

		return oFireEventClientSide(ecx, pEvent);
	};

	float __stdcall hookViewModelFov()
	{
		static auto ofunc = clientmode_hook.get_original<ViewModelFov>(index::ViewModel);
		return Vars.misc_viewmodelfov > 0 ? Vars.misc_viewmodelfov : ofunc();
	}

	void __stdcall hookOverrideView(CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<OverrideView>(index::OverrideView);

		CMisc::Get().OverrideView(vsView);
		CMisc::Get().NoView(vsView);

		ofunc(Interfaces::ClientMode, vsView);
	}

	typedef bool(__thiscall *svc_get_bool_t)(PVOID);

	static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleA(XorStr("client_panorama.dll")), XorStr("85 C0 75 30 38 86"));

	bool __fastcall hookSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto ofunc = sv_cheats.get_original<svc_get_bool_t>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}
}