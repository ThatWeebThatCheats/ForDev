#pragma once

#include <string>
#include "../Helpers/singleton.hpp"
#include "../ImGui/imgui.h"

#include <D3dx9tex.h>
#pragma comment (lib,"D3dx9.lib")

struct IDirect3DDevice9;

class CMenu : public Singleton<CMenu>
{
public:
	void InvalidateDeviceObjects();
	void CreateDeviceObjects();
	void EndScene(IDirect3DDevice9 * device);
	void Shutdown();
private:
	void RenderSkinsTab();
	void RenderAATab();
	void RenderRagebotTab();
	void RenderVisualsTab();
	void RenderMiscTab();
	void RenderInfoTab();
	void MainFrame();
	void SpectatorListFrame();
	bool Initialize(IDirect3DDevice9 * device);
	void ImColors(ImGuiStyle & style);
	void ImStyles(ImGuiStyle & style);
};