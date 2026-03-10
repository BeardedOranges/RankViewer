// Credits: Bakkes: MMR rounding, being a cool guy. CinderBlock: overall big help <3. mega: ] his SessionStats plugin was a really helpful reference. Others: savior, Martinn, Simple_AOB, HalfwayDead, ItsBrank, CWO333.
// New Credits: Martin for the clean-up, thanks so much! I know it was a mess before, I suck at coding :D
// New New Credits: I don't even know anymore so many people have helped, love this community everyone is so helpful <3

// To Do:
// - Fix aspect ratio being off for screens larger than 1920x1080p.
// - Redo the hide UI when friends list is open feature.
// - Add support for the quads and heatseeker playlists.

#include "pch.h"
#include "RankViewer.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include "bakkesmod/wrappers/GuiManagerWrapper.h"
#include "utils/parser.h"
#include <nlohmann/json.hpp>

BAKKESMOD_PLUGIN(RankViewer, "Rank Viewer", "2.2.0", 0)

// Called when the plugin is loaded.
void RankViewer::onLoad() {
    // I didn't write this part, no idea how it works. It's for grabbing the mmr from the game.
    gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
        cvarManager->executeCommand("togglemenu " + GetMenuName());
    }, 1);

    // Setting for if the plugin is enabled.
	cvarManager->registerCvar("rankviewer_enabled", "1", "Enable or Disable the Rank Viewer Plugin", true, true, 0, true, 1, true);

    // Called when game ends.
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", std::bind(&RankViewer::OnStatsScreen, this, std::placeholders::_1));

    // Called when you leave the stats screen or exit games.
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&RankViewer::OnGameLeave, this, std::placeholders::_1));

    // Called when you open or close menus, such as the friends list tab.
    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_MenuStack_TA.ButtonTriggered",  std::bind(&RankViewer::OnFriendScreen, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_friendsOpenName = FNameOJ(gameWrapper->GetFNameIndexByString("friendsButton"));
    m_friendsCloseName = FNameOJ(gameWrapper->GetFNameIndexByString("closeButton"));

    // Puts in the unranked icon as a placeholder for the three images in case something goes wrong later on.
    const std::filesystem::path unrankedPath = (gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / "0.tga");
    m_previousRankImg = std::make_shared<ImageWrapper>(unrankedPath, false, true);
    m_currentRankImg = std::make_shared<ImageWrapper>(unrankedPath, false, true);
    m_nextRankImg = std::make_shared<ImageWrapper>(unrankedPath, false, true);

    // Screen resolution.
    m_screenSize = gameWrapper->GetScreenSize();
}

// Called when unloading the plugin.
void RankViewer::onUnload() {
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet");
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");
    gameWrapper->UnhookEvent("Function TAGame.GFxData_MenuStack_TA.ButtonTriggered");
    gameWrapper->UnregisterDrawables();
}

// Decides if it should render or not.
void RankViewer::Render() {
    // Only displays if the user has the plugin enabled.
    m_pluginEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();

    if (!m_pluginEnabled) {
        return;
    }

    if (m_friendsListOpen) {
        return;
    }

    // Makes sure you are in a game.
    if (gameWrapper->IsInOnlineGame()) {
        if (m_drawCanvas) {
            RenderImGui();
        }
    }
    else {
        m_drawCanvas = false;
    }
}

// The actual rendering of ImGui.
void RankViewer::RenderImGui() {
    if ((m_screenSize.X <= 0) || (m_screenSize.Y <= 0)) {
        return; // Just in case this function is called before screen size is grabbed, divide by zero = crash.
    }

    // Percentages for converting to a non-1080p screen.
    const float xPercent = (static_cast<float>(m_screenSize.X) / 1920.0f);
    const float yPercent = (static_cast<float>(m_screenSize.Y) / 1080.0f);
    const float upperBound = 290.0f;
    const float lowerBound = 835.0f;

    // The ImGui window allows the quads to show on the screen.
    ImVec2 windowPos = ImVec2((1660 * xPercent), 0);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((m_screenSize.X - (1660.0f * xPercent) + (10.0f * xPercent)), (m_screenSize.Y + (10.0f * yPercent))));

    auto gui = gameWrapper->GetGUIManager();
    m_fontBig = gui.GetFont("PantonBig");

    // Early out if the window is collapsed, as an optimization.
    if (!ImGui::Begin(m_menuTitle.c_str(), &m_windowOpen, (ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar))) {
        ImGui::End();
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Sidebar.
    ImVec2 centerPoint = ImVec2(1920, 950);
    drawList->AddQuadFilled(ImVec2((xPercent * centerPoint.x), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 45)), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 45)), (yPercent * (centerPoint.y - 820))),
        ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 820))),
        IM_COL32_BLACK);
    
    // Lower box.
    centerPoint = ImVec2(1875, 950);
    drawList->AddQuadFilled(ImVec2((xPercent * centerPoint.x), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 180)), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 205)), (yPercent * (centerPoint.y - 45))),
        ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 45))),
        IM_COL32_BLACK);
    drawList->AddTriangleFilled(ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 45))),
        ImVec2((xPercent * (centerPoint.x - 35)), (yPercent * (centerPoint.y - 45))),
        ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 80))),
        IM_COL32_BLACK);

    // Displays previous rank text and fonts.
    if (m_fontBig) {
        float fontSize = (35.0f * xPercent);
        ImGui::PushFont(m_fontBig);
        drawList->AddText(m_fontBig, fontSize, ImVec2((xPercent * (centerPoint.x - 155)), (yPercent * (centerPoint.y - 40))), ImU32(m_white), std::to_string(m_beforeUpperMMR).c_str());
        ImGui::PopFont();

        fontSize = (25.0f * xPercent);
        ImGui::PushFont(m_fontBig);
        drawList->AddText(m_fontBig, fontSize, ImVec2((xPercent * (centerPoint.x - 85)), (yPercent * (centerPoint.y - 32))), ImU32(m_white), "MMR");
        ImGui::PopFont();

        fontSize = (20.0f * xPercent);
        ImGui::PushFont(m_fontBig);
        drawList->AddText(m_fontBig, fontSize, ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 75))), ImU32(m_white), m_previousName.c_str());
        ImGui::PopFont();
    }

    // Displays previous rank image.
    if (m_previousRankImg && m_previousRankImg->IsLoadedForImGui()) {
        if (auto prevRankTex = m_previousRankImg->GetImGuiTex()) {
            Vector2F prevRankRect = m_previousRankImg->GetSizeF();
            ImGui::SetCursorPos(ImVec2(((xPercent * (centerPoint.x - 10.0f)) - windowPos.x), (yPercent * (centerPoint.y - 50.0f))));
            ImGui::Image(prevRankTex, ImVec2((prevRankRect.X * 0.19f * xPercent), (prevRankRect.Y * 0.19f * yPercent)));
        }
    }

    // Upper box.
    centerPoint = ImVec2(1875, 175);
    drawList->AddQuadFilled(ImVec2((xPercent * centerPoint.x), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 180)), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 205)), (yPercent * (centerPoint.y - 45))),
        ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 45))),
        IM_COL32_BLACK);
    drawList->AddTriangleFilled(ImVec2((xPercent * centerPoint.x), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 35)), (yPercent * centerPoint.y)),
        ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y + 35))),
        IM_COL32_BLACK);

    // Displays next rank text and fonts.
    if (m_fontBig) {
        float fontSize = (35.0f * xPercent);
        ImGui::PushFont(m_fontBig);
        drawList->AddText(m_fontBig, fontSize, ImVec2((xPercent * (centerPoint.x - 155)), (yPercent * (centerPoint.y - 40))), ImU32(m_white), std::to_string(m_nextLowerMMR).c_str());
        ImGui::PopFont();

        fontSize = (25.0f * xPercent);
        ImGui::PushFont(m_fontBig);
        drawList->AddText(m_fontBig, fontSize, ImVec2((xPercent * (centerPoint.x - 85)), (yPercent * (centerPoint.y - 32))), ImU32(m_white), "MMR");
        ImGui::PopFont();

        fontSize = (20.0f * xPercent);
        ImGui::PushFont(m_fontBig);
        drawList->AddText(m_fontBig, fontSize, ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 40))), ImU32(m_white), m_nextName.c_str());
        ImGui::PopFont();
    }

    // Displays next rank image.
    if (m_nextRankImg && m_nextRankImg->IsLoadedForImGui()) {
        if (auto nextRankTex = m_nextRankImg->GetImGuiTex()) {
            Vector2F nextRankRect = m_nextRankImg->GetSizeF();
            ImGui::SetCursorPos(ImVec2(((xPercent * (centerPoint.x - 10)) - windowPos.x), (yPercent * (centerPoint.y - 25))));
            ImGui::Image(nextRankTex, ImVec2((nextRankRect.X * 0.19f * xPercent), (nextRankRect.Y * 0.19f * yPercent)));
        }
    }

    // Determines where the middle box should be (y position) based on upper and lower bounds.
    int32_t yPos = static_cast<int32_t>((upperBound + (((m_nextLowerMMR - m_currentMMR) / (m_nextLowerMMR - m_beforeUpperMMR)) * (lowerBound - upperBound))));

    if (yPos < upperBound) {
        yPos = upperBound;
    }
    else if (yPos > lowerBound) {
        yPos = lowerBound;
    }
    
    // Middle box.
    centerPoint = ImVec2(1875, yPos);
    drawList->AddQuadFilled(ImVec2((xPercent * centerPoint.x), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 180)), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 205)), (yPercent * (centerPoint.y - 45))),
        ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 45))),
        m_lightBlue);
    drawList->AddQuadFilled(ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y + 35))),
        ImVec2((xPercent * (centerPoint.x + 45)), (yPercent * (centerPoint.y + 35))),
        ImVec2((xPercent * (centerPoint.x + 45)), (yPercent * (centerPoint.y - 79))),
        ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 79))),
        m_lightBlue);
    drawList->AddTriangleFilled(ImVec2((xPercent * centerPoint.x), (yPercent * centerPoint.y)),
        ImVec2((xPercent * (centerPoint.x - 35)), (yPercent * centerPoint.y)),
        ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y + 35))),
        m_lightBlue);
    drawList->AddTriangleFilled(ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 45))),
        ImVec2((xPercent * (centerPoint.x - 35)), (yPercent * (centerPoint.y - 45))),
        ImVec2((xPercent * centerPoint.x), (yPercent * (centerPoint.y - 80))),
        m_lightBlue);

    // Displays current rank text and fonts.
    if (m_fontBig) {
        float fontSize = (35.0f * xPercent);
        ImGui::PushFont(m_fontBig);
        drawList->AddText(m_fontBig, fontSize, ImVec2((xPercent * (centerPoint.x - 155)), (yPercent * (centerPoint.y - 40))), ImU32(m_darkBlue), std::to_string(static_cast<int32_t>(std::round(m_currentMMR))).c_str());
        ImGui::PopFont();

        fontSize = (25.0f * xPercent);
        ImGui::PushFont(m_fontBig);
        drawList->AddText(m_fontBig, fontSize, ImVec2((xPercent * (centerPoint.x - 85)), (yPercent * (centerPoint.y - 32))), ImU32(m_darkBlue), "MMR");
        ImGui::PopFont();

        fontSize = (20.0f * xPercent);
        ImGui::PushFont(m_fontBig);
        drawList->AddText(m_fontBig, fontSize, ImVec2((xPercent * (centerPoint.x)), (yPercent * (centerPoint.y - 65))), ImU32(m_darkBlue), m_currentName.c_str());
        ImGui::PopFont();
    }

    // Displays current rank image.
    if (m_currentRankImg && m_currentRankImg->IsLoadedForImGui()) {
        if (auto currentRankTex = m_currentRankImg->GetImGuiTex()) {
            Vector2F currentRankRect = m_currentRankImg->GetSizeF();
            ImGui::SetCursorPos(ImVec2(((xPercent * (centerPoint.x - 10.0f)) - windowPos.x), (yPercent * (centerPoint.y - 40.0f))));
            ImGui::Image(currentRankTex, ImVec2((currentRankRect.X * 0.19f * xPercent), (currentRankRect.Y * 0.19f * yPercent)));
        }
    }

    ImGui::End();

    if (!m_windowOpen) {
        cvarManager->executeCommand("togglemenu " + GetMenuName());
    }
}

// Name of the menu that is used to toggle the window.
std::string RankViewer::GetMenuName() {
    return m_menuName;
}

// Title to give the menu.
std::string RankViewer::GetMenuTitle() {
    return m_menuTitle;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context.
void RankViewer::SetImGuiContext(uintptr_t ctx) {
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
    gameWrapper->GetGUIManager().LoadFont("PantonBig", "Panton-LightCaps.otf", 32);
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game.
bool RankViewer::ShouldBlockInput() {
    return (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard);
}

// Return true if window should be interactive.
bool RankViewer::IsActiveOverlay() {
    return false;
}

// Called when the window is opened.
void RankViewer::OnOpen() {
    m_windowOpen = true;
}

// Called when the window is closed.
void RankViewer::OnClose() {
    m_windowOpen = false;
}

// Called when the game ends.
void RankViewer::OnStatsScreen(std::string eventName) {
    m_pluginEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();

    if (!m_pluginEnabled) {
        return;
    }

    // Getting the playlist and the users unique id.
    MMRWrapper mw = gameWrapper->GetMMRWrapper();
    m_uniqueID = gameWrapper->GetUniqueID();
    m_currentPlaylist = mw.GetCurrentPlaylist();

    // Gets the screen size just in case it changed from the last time.
    m_screenSize = gameWrapper->GetScreenSize();

    // The friends list automatically closes when the game finishes.
    m_friendsListOpen = false;

    // Getting the mmr, and after that all of the rank information needed.
    if (mw.IsRanked(m_currentPlaylist)) {
        // Makes sure the mmr updates
        CheckMMR(5);
    }
}

// Called when you go back to the main menu or leave game.
void RankViewer::OnGameLeave(std::string eventName) {
    // Removes canvas if you quit the stats screen.
    m_drawCanvas = false;
    m_friendsListOpen = false;
}

// Called when you open or close the friend tab.
void RankViewer::OnFriendScreen(ActorWrapper caller, void* params, const std::string& functionName) {
    // Temporarily disabling this feature until I get it working better.

    //if (params) 
    //{
    //    FNameOJ menuName = *reinterpret_cast<FNameOJ*>(params);

    //    if (menuName == m_friendsOpenName) {
    //        m_friendsListOpen = true;
    //        //cvarManager->log(to_string(isFriendOpen));
    //    }
    //    else if (menuName == m_friendsCloseName) {
    //        m_friendsListOpen = false;
    //        //cvarManager->log(to_string(isFriendOpen));
    //    }
    //}
}

bool RankViewer::IsValidPlaylist(int32_t playlistId) {
    return (std::find(m_supportedPlaylists.begin(), m_supportedPlaylists.end(), playlistId) != m_supportedPlaylists.end());
}

bool RankViewer::IsValidRank(int32_t rank) {
    return ((rank >= RANK_UNRANKED) && (rank <= RANK_SUPERSONICLEGEND));
}

bool RankViewer::IsValidDiv(int32_t div) {
    return ((div >= DIVISION_ONE) && (div <= DIVISION_FOUR));
}

std::string RankViewer::GetRankName(int32_t rank) {
    switch (rank) {
    case RANK_UNRANKED:
        return "Unranked";
    case RANK_BRONZE_I:
        return "Bronze I";
    case RANK_BRONZE_II:
        return "Bronze II";
    case RANK_BRONZE_III:
        return "Bronze III";
    case RANK_SILVER_I:
        return "Silver I";
    case RANK_SILVER_II:
        return "Silver II";
    case RANK_SILVER_III:
        return "Silver III";
    case RANK_GOLD_I:
        return "Gold I";
    case RANK_GOLD_II:
        return "Gold II";
    case RANK_GOLD_III:
        return "Gold III";
    case RANK_PLATINUM_I:
        return "Platinum I";
    case RANK_PLATINUM_II:
        return "Platinum II";
    case RANK_PLATINUM_III:
        return "Platinum III";
    case RANK_DIAMOND_I:
        return "Diamond I";
    case RANK_DIAMOND_II:
        return "Diamond II";
    case RANK_DIAMOND_III:
        return "Diamond III";
    case RANK_CHAMPION_I:
        return "Champion I";
    case RANK_CHAMPION_II:
        return "Champion II";
    case RANK_CHAMPION_III:
        return "Champion III";
    case RANK_GRANDCHAMPION_I:
        return "Grand Champion I";
    case RANK_GRANDCHAMPION_II:
        return "Grand Champion II";
    case RANK_GRANDCHAMPION_III:
        return "Grand Champion III";
    case RANK_SUPERSONICLEGEND:
        return "Super Sonic Legend";
    default:
        return "RANK ERROR";
    }
}

std::string RankViewer::GetDivName(int32_t div) {
    switch (div) {
    case DIVISION_ONE:
        return "I";
    case DIVISION_TWO:
        return "II";
    case DIVISION_THREE:
        return "II";
    case DIVISION_FOUR:
        return "IV";
    default:
        return "DIV ERROR";
    }
}

// Converts rank and div into the usable string that displays on the screen.
std::string RankViewer::GetRankDivName(int32_t rank, int32_t div) {
    if (!IsValidRank(rank)) { // I don't get why rank is needed as an argument but I'm leaving this here anyway.
        return "RANK ERROR";
    }

    if (!IsValidDiv(div)) {
        return "DIV ERROR";
    }

    return ("DIV " + GetDivName(div));
    //std::string rankName = GetRankName(rank);

    //if ((rank == RANK_UNRANKED) || (rank == RANK_SUPERSONICLEGEND)) {
    //    return rankName; // Unranked and super sonic legend don't have divisions.
    //}

    //return (rankName + " DIV " + GetDivName(div));
}

// Converts information into mmr. Format is directly from game (mode is 11-34, rank is 0-22, div is 0-3).
// Upper limit is to get the upper part of the range, setting it to false gets the lower part of the mmr range.
// Oranges you should really cache this instead of reading and parsing it every single time "CheckMMR" is called - Brank.
int32_t RankViewer::Unranker(int32_t playlistId, int32_t rank, int32_t div, bool upperLimit) {
    const std::string fileName = (std::to_string(playlistId) + ".json");
    const std::filesystem::path rankJSON = (gameWrapper->GetDataFolder() / "RankViewer" / "RankNumbers" / fileName); // Gets the correct json from the folder.

    if (std::filesystem::exists(rankJSON))
    {
        std::string limit = (upperLimit ? "maxMMR" : "minMMR");

        // Stores the json.
        std::ifstream file(rankJSON);
        nlohmann::json j = nlohmann::json::parse(file);

        // Gets the correct mmr number.
        return j["data"]["data"][((rank - 1) * 4) + (div + 1)][limit];

        /*
        // This was an old attempt to fix the jsons. They currently have an issue where if there is missing spots in the database, it crashes. So for the few spots I have to manually add numbers to the json which is a pain
        // Below here also crashed the game, but leaving it here in case I want another attempt at fixing it
        int mmrNum = -1;

        for (int id = 0; id <= j["data"]["data"].size(); id++) {
            if (j["data"]["data"][id]["tier"] == rank && j["data"]["data"][id]["division"] == div) {
                mmrNum = j["data"]["data"][id][limit];
            }
        }

        return mmrNum; // return here
        */
    }

    return -1;
}

// Gets mmr and rank information for displaying
void RankViewer::CheckMMR(int32_t retryCount) {
    m_pluginEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();

    if (!m_pluginEnabled) {
        return;
    }

    // The updateMMR section is all from mega's plugin: SessionStats. Please view it here, its great :) https://bakkesplugins.com/plugins/view/39

    ServerWrapper sw = gameWrapper->GetOnlineGame();

    if (sw.IsNull() || !sw.IsOnlineMultiplayer() || gameWrapper->IsInReplay()) {
        return;
    }

    if ((retryCount < 0) || (retryCount > 20)) {
        return;
    }

    if (m_currentPlaylist > 0) {
        gameWrapper->SetTimeout([retryCount, this](GameWrapper* gameWrapper) {
            m_gotNewMMR = false;

            while (!m_gotNewMMR) {
                if (gameWrapper->GetMMRWrapper().IsSynced(m_uniqueID, m_currentPlaylist) && !gameWrapper->GetMMRWrapper().IsSyncing(m_uniqueID)) {

                    // Makes sure it is one of the supported playlists to prevent crashes.

                    if (!IsValidPlaylist(m_currentPlaylist)) {
                        return;
                    }

                    // Getting the mmr.
                    m_currentMMR = gameWrapper->GetMMRWrapper().GetPlayerMMR(m_uniqueID, m_currentPlaylist);
                    m_gotNewMMR = true;

                    MMRWrapper mw = gameWrapper->GetMMRWrapper();

                    // The SkillRank has information about the players rank.
                    SkillRank userRank = mw.GetPlayerRank(m_uniqueID, m_currentPlaylist);

                    // Getting the player rank information into separate variables.
                    m_currentRank = userRank.Tier;
                    m_currentDiv = userRank.Division;

                    // Converts the rank and div into the division with the roman numeral (I, II, III, IV).
                    m_currentName = GetRankDivName(m_currentRank, m_currentDiv);

                    // Gets and loads the rank icon for your current rank from the RankViewer folder
                    std::string fileName = (std::to_string(m_currentRank) + ".tga");
                    const std::filesystem::path currentPath = (gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName);
                    m_currentRankImg = std::make_shared<ImageWrapper>(currentPath, false, true);

                    // This all checks for different scenarios where it won't be the default method of displaying
                    if (m_currentRank <= RANK_UNRANKED) { // --- When still in placement matches -- 
                        // For placement shows from bronze 1 and supersonic legend.
                        m_previousRank = RANK_BRONZE_I;
                        m_nextRank = RANK_SUPERSONICLEGEND;

                        m_nextLowerMMR = Unranker(m_currentPlaylist, m_nextRank, DIVISION_ONE, true); // Div has to be I (0) since ssl doesn't have divisions.
                        m_nextName = GetRankDivName(RANK_SUPERSONICLEGEND, DIVISION_ONE);

                        m_beforeUpperMMR = Unranker(m_currentPlaylist, m_previousRank, DIVISION_ONE, false);
                        m_previousName = GetRankDivName(RANK_UNRANKED, DIVISION_ONE); // This inputs the unranked name since it just won't show the division number.
                    }
                    else if ((m_currentRank == RANK_BRONZE_I) && (m_currentDiv == DIVISION_ONE)) {
                        // It just shows the bronze 1 div 1 lower limit on the bottom and bronze 1 div 2 on top.
                        m_nextRank = m_currentRank;
                        m_previousRank = m_currentRank;
                        m_nextDiv = (m_currentDiv + 1);
                        m_previousDiv = DIVISION_ONE;

                        // Finds the mmr for that div and rank.
                        m_nextLowerMMR = Unranker(m_currentPlaylist, m_nextRank, m_nextDiv, false);
                        m_nextName = GetRankDivName(m_nextRank, m_nextDiv);

                        m_beforeUpperMMR = Unranker(m_currentPlaylist, m_previousRank, m_previousDiv, false);
                        m_previousName = GetRankDivName(m_previousRank, m_previousDiv);
                    }
                    else if (m_currentRank == RANK_SUPERSONICLEGEND) {
                        // Shows the ssl upper limit on top and gc 3 div 4 on bottom.
                        m_nextRank = m_currentRank;
                        m_previousRank = (m_currentRank - 1);
                        m_nextDiv = m_currentDiv;
                        m_previousDiv = DIVISION_FOUR;

                        // Finds the mmr for that div and rank.
                        m_nextLowerMMR = Unranker(m_currentPlaylist, m_nextRank, m_nextDiv, true);
                        m_nextName = GetRankDivName(m_nextRank, m_nextDiv);

                        m_beforeUpperMMR = Unranker(m_currentPlaylist, m_previousRank, m_previousDiv, true);
                        m_previousName = GetRankDivName(m_previousRank, m_previousDiv);
                    }
                    else {
                        // Finds out what div is above and below you.
                        if (m_currentDiv == DIVISION_ONE) {
                            m_nextRank = m_currentRank;
                            m_previousRank = (m_currentRank - 1);
                            m_nextDiv = (m_currentDiv + 1);
                            m_previousDiv = DIVISION_FOUR;

                        }
                        else if (m_currentDiv == DIVISION_FOUR) {
                            m_nextRank = (m_currentRank + 1);
                            m_previousRank = m_currentRank;
                            m_nextDiv = DIVISION_ONE;
                            m_previousDiv = (m_currentDiv - 1);
                        }
                        else {
                            m_nextRank = m_currentRank;
                            m_previousRank = m_currentRank;
                            m_nextDiv = (m_currentDiv + 1);
                            m_previousDiv = (m_currentDiv - 1);
                        }

                        // Finds the mmr for that div and rank.
                        m_nextLowerMMR = Unranker(m_currentPlaylist, m_nextRank, m_nextDiv, false);
                        m_nextName = GetRankDivName(m_nextRank, m_nextDiv);

                        m_beforeUpperMMR = Unranker(m_currentPlaylist, m_previousRank, m_previousDiv, true);
                        m_previousName = GetRankDivName(m_previousRank, m_previousDiv);
                    }

                    // Gets correct rank icons from folder for before and after ranks.
                    fileName = (std::to_string(m_previousRank) + ".tga");
                    const std::filesystem::path previousPath = (gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName);
                    m_previousRankImg = std::make_shared<ImageWrapper>(previousPath, false, true);

                    fileName = (std::to_string(m_nextRank) + ".tga");
                    const std::filesystem::path nextPath = (gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName);
                    m_nextRankImg = std::make_shared<ImageWrapper>(nextPath, false, true);

                    // Lets rank viewer display.
                    m_drawCanvas = true;
                }

                // Failsafe.
                if (!m_gotNewMMR && (retryCount > 0)) {
                    gameWrapper->SetTimeout([retryCount, this](GameWrapper* gameWrapper) {
                        this->CheckMMR(retryCount - 1);
                    }, 0.5f);
                }
                else {
                    return;
                }
            }
        }, 3);
    }
}