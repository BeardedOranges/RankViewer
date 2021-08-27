#pragma once
#pragma comment( lib, "pluginsdk.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "imgui/imgui.h"

using namespace std;

class RankViewer : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
private:
	// Steam/Epic id
	UniqueIDWrapper uniqueID;

	bool drawCanvas, isEnabled;

	void CheckMMR(int retryCount);
	bool gotNewMMR;

	// Friends Menu
	bool isFriendOpen = false;
	struct FName2
	{
		int32_t Index;
		int32_t Instance;
	};
	FName2 friendsOpen, friendsClose;

	int userPlaylist, userDiv, userTier, upperTier, lowerTier, upperDiv, lowerDiv, nextLower, beforeUpper, yPos;
	float userMMR = 0;

	// Div numbers are stored in these
	string nameCurrent, nameNext, nameBefore;
	string nextDiff, beforeDiff;

	// Colors for the rank viewer graphics
	ImColor lightBlue = ImVec4(0.862745098f, 0.968627451f, 1.0f, 1.0f);
	ImColor darkBlue = ImVec4(0.0117647059f, 0.3803921569f, 0.5647058824f, 1.0f);
	ImColor white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	// Images n fonts
	std::shared_ptr<ImageWrapper> beforeRank, currentRank, nextRank;
	ImFont* fontBig;

	// Imgui stuff
	virtual void onLoad();
	virtual void onUnload();
	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "RankViewer";
	Vector2 screenSize;
	//float safeZone;
	//float uiScale;
	void Render() override;
	void RenderImGui();
	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;

	// Converts into mmr
	int unranker(int mode, int rank, int div, bool upperLimit);

	// Rocket League Events
	void StatsScreen(std::string eventName);
	void loadMenu(std::string eventName);
	void friendScreen(ActorWrapper caller, void* params, const std::string& functionName);

	int rankedPlaylists[8] = {	10, // Ones
								11, // Twos
								13, // Threes
								27, // Hoops
								28, // Rumble
								29, // Dropshot
								30, // Snowday
								34 // Psynoix Tournaments
	};
};