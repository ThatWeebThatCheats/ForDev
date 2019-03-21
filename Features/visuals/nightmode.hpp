#pragma once
#include "../../options.hpp"

class CNightmode : public Singleton<CNightmode>
{
public:
	std::string OldSkyname = "";
	bool NightmodeDone = true;
	void Apply();
};

class CFullBright : public Singleton<CFullBright>
{
public:
	std::string OldSkyname = "";
	bool FullbrightDone = true;
	void Apply();
};