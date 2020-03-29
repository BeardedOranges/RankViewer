#pragma once
#pragma comment( lib, "bakkesmod.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"

class RankViewer : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	SteamID mySteamID;
	bool drawCanvas, isEnabled, gotNewMMR, isPlacement;
	int userPlaylist, userDiv, userTier, upperTier, lowerTier, upperDiv, lowerDiv, nextLower, beforeUpper;
	float userMMR;
	string nameCurrent, nameNext, nameBefore, nextDiff, beforeDiff;
public:
	virtual void onLoad();
	virtual void onUnload();
	
	void Render(CanvasWrapper canvas);
	void StatsScreen(std::string eventName);
	void loadMenu(std::string eventName);
	void CheckMMR(int retryCount);
};