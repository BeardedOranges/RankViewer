#pragma once
#pragma comment( lib, "pluginsdk.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "imgui/imgui.h"

// Ranks aka tiers.
static constexpr int32_t RANK_UNRANKED = 0;
static constexpr int32_t RANK_BRONZE_I = 1;
static constexpr int32_t RANK_BRONZE_II = 2;
static constexpr int32_t RANK_BRONZE_III = 3;
static constexpr int32_t RANK_SILVER_I = 4;
static constexpr int32_t RANK_SILVER_II = 5;
static constexpr int32_t RANK_SILVER_III = 6;
static constexpr int32_t RANK_GOLD_I = 7;
static constexpr int32_t RANK_GOLD_II = 8;
static constexpr int32_t RANK_GOLD_III = 9;
static constexpr int32_t RANK_PLATINUM_I = 10;
static constexpr int32_t RANK_PLATINUM_II = 11;
static constexpr int32_t RANK_PLATINUM_III = 12;
static constexpr int32_t RANK_DIAMOND_I = 13;
static constexpr int32_t RANK_DIAMOND_II = 14;
static constexpr int32_t RANK_DIAMOND_III = 15;
static constexpr int32_t RANK_CHAMPION_I = 16;
static constexpr int32_t RANK_CHAMPION_II = 17;
static constexpr int32_t RANK_CHAMPION_III = 18;
static constexpr int32_t RANK_GRANDCHAMPION_I = 19;
static constexpr int32_t RANK_GRANDCHAMPION_II = 20;
static constexpr int32_t RANK_GRANDCHAMPION_III = 21;
static constexpr int32_t RANK_SUPERSONICLEGEND = 22;

// Divisions.
static constexpr int32_t DIVISION_ONE = 0;
static constexpr int32_t DIVISION_TWO = 1;
static constexpr int32_t DIVISION_THREE = 2;
static constexpr int32_t DIVISION_FOUR = 3;

// Get it? OJ for orange juice.
struct FNameOJ
{
	int32_t FNameEntryId;
	int32_t InstanceNumber;

public:
	FNameOJ() : FNameEntryId(-1), InstanceNumber(0) {}
	FNameOJ(int32_t id) : FNameEntryId(id), InstanceNumber(0) {}
	FNameOJ(const FNameOJ& name) : FNameEntryId(name.FNameEntryId), InstanceNumber(name.InstanceNumber) {}
	~FNameOJ() {}

public:
	FNameOJ& operator=(const FNameOJ& other)
	{
		FNameEntryId = other.FNameEntryId;
		InstanceNumber = other.InstanceNumber;
		return *this;
	}

	bool operator==(const FNameOJ& other) const
	{
		return ((FNameEntryId == other.FNameEntryId) && (InstanceNumber == other.InstanceNumber));
	}

	bool operator!=(const FNameOJ& other) const
	{
		return !(*this == other);
	}
};

class RankViewer : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
private:
	std::vector<int32_t> m_supportedPlaylists = {
		10, // Ones.
		11, // Twos.
		13, // Threes.
		27, // Hoops.
		28, // Rumble.
		29, // Dropshot.
		30, // SnowDay.
		34, // Psyonix Tournaments.
		//61, // Quads (Not supported currently!)
		//63, // Heatseeker Doubles (Not supported currently!)
	};

	UniqueIDWrapper m_uniqueID;
	int32_t m_currentPlaylist = 0;
	bool m_pluginEnabled = false;
	bool m_drawCanvas = false;
	bool m_gotNewMMR = false;

	// Friend list menu.
	bool m_friendsListOpen = false;
	FNameOJ m_friendsOpenName;
	FNameOJ m_friendsCloseName;

	// The users rank, division, and current playlist.
	float m_currentMMR = 0.0f;
	int32_t m_previousRank = RANK_UNRANKED;
	int32_t m_previousDiv = DIVISION_ONE;
	int32_t m_currentRank = RANK_UNRANKED;
	int32_t m_currentDiv = DIVISION_ONE;
	int32_t m_nextRank = RANK_UNRANKED;
	int32_t m_nextDiv = DIVISION_ONE;
	int32_t m_nextLowerMMR = 0;
	int32_t m_beforeUpperMMR = 0;

	// Div numbers are stored in these for ImGui.
	std::string m_previousName;
	std::string m_currentName;
	std::string m_nextName;

	// ImGui colors for the rank viewer graphics.
	ImColor m_lightBlue = ImVec4(0.862745098f, 0.968627451f, 1.0f, 1.0f);
	ImColor m_darkBlue = ImVec4(0.0117647059f, 0.3803921569f, 0.5647058824f, 1.0f);
	ImColor m_white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	// ImGui images and fonts.
	std::shared_ptr<ImageWrapper> m_previousRankImg;
	std::shared_ptr<ImageWrapper> m_currentRankImg;
	std::shared_ptr<ImageWrapper> m_nextRankImg;
	ImFont* m_fontBig = nullptr;

	// ImGui window stuff.
	bool m_windowOpen = false;
	std::string m_menuName = "RankViewer";
	std::string m_menuTitle = "RankViewer";
	Vector2 m_screenSize = { 0, 0 };

	virtual void onLoad();
	virtual void onUnload();
	void Render() override;
	void RenderImGui();
	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;

	// Rocket League hooks.
	void OnStatsScreen(std::string eventName);
	void OnGameLeave(std::string eventName);
	void OnFriendScreen(ActorWrapper caller, void* params, const std::string& functionName);

	// Helper functions and utils.
	bool IsValidPlaylist(int32_t playlistId);
	bool IsValidRank(int32_t rank);
	bool IsValidDiv(int32_t div);
	std::string GetRankName(int32_t rank);
	std::string GetDivName(int32_t div);
	std::string GetRankDivName(int32_t rank, int32_t div); // For use specifically in ImGui.
	int32_t Unranker(int32_t playlistId, int32_t rank, int32_t div, bool upperLimit); // Converts into mmr.
	void CheckMMR(int32_t retryCount);
};