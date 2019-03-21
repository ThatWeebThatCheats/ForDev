#pragma once
#include <string>
#include "config_manager.hpp"
#include "vfunc_hook.hpp"
#include "utils.hpp"
#include "draw_manager.hpp"

class CNotification
{
public:
	void Draw();

private:
};

extern CNotification g_Notification;
