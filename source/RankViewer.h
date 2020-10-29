#pragma once
#pragma comment( lib, "bakkesmod.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "RankEnums.h"
#include "PlaylistData.h"
#include <string>

class RankViewer : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	SteamID mySteamID;
	bool drawCanvas, isEnabled, gotNewMMR, isPlacement;
	int userPlaylist, userDiv, userTier, upperTier, lowerTier, upperDiv, lowerDiv, nextLower, beforeUpper;
	float userMMR;
	std::string nameCurrent, nameNext, nameBefore, nextDiff, beforeDiff;
public:
	virtual void onLoad();
	virtual void onUnload();

	int unranker(int mode, int rank, int div, bool upperLimit);
	DivisionData GetDivisionData(Playlist mode, Rank rank, int div);
	void DebugGetDivisionData(std::vector<std::string> args);

	
	void Render(CanvasWrapper canvas);
	void StatsScreen(std::string eventName);
	void loadMenu(std::string eventName);
	void CheckMMR(int retryCount);
};