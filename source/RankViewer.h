#pragma once
#pragma comment( lib, "pluginsdk.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "RankEnums.h"
#include "PlaylistData.h"
#include "imgui/imgui.h"

using namespace std;

class RankViewer : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
private:
	UniqueIDWrapper uniqueID;
	bool drawCanvas, isEnabled, gotNewMMR, isPlacement;
	bool isFriendOpen = false;
	int userPlaylist, userDiv, userTier, upperTier, lowerTier, upperDiv, lowerDiv, nextLower, beforeUpper, yPos;
	float userMMR = 0;
	string nameCurrent, nameNext, nameBefore, nextDiff, beforeDiff;

	FName2 friendsOpen, friendsClose;

	std::shared_ptr<ImageWrapper> beforeRank, currentRank, nextRank;

	Vector2 screenSize;
	//float safeZone;
	//float uiScale;
public:
	virtual void onLoad();
	virtual void onUnload();

	// Functions and shit
	int unranker(int mode, int rank, int div, bool upperLimit);
	//DivisionData GetDivisionData(Playlists mode, Rank rank, int div);
	//void DebugGetDivisionData(std::vector<std::string> args);

	//void RenderCanvas(CanvasWrapper canvas);

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "RankViewer";
	
	// Imgui shit
	void Render() override;
	void RenderImGui();
	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;

	ImFont* fontBig;

	// Need new light and dark blue color tmrw lol good luck
	ImColor lightBlue = ImVec4(0.862745098f, 0.968627451f, 1.0f, 1.0f);
	ImColor darkBlue = ImVec4(0.0117647059f, 0.3803921569f, 0.5647058824f, 1.0f);
	ImColor white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	// i dont even know
	void StatsScreen(std::string eventName);
	void loadMenu(std::string eventName);
	void friendScreen(ActorWrapper caller, void* params, const std::string& functionName);
	void CheckMMR(int retryCount);

	int rankedPlaylists[8] = { 10, // Ones
					11, // Twos
					13, // Threes
					27, // Hoops
					28, // Rumble
					29, // Dropshot
					30, // Snowday
					34 // Psynoix Tournaments
	};
};