// Credits: Bakkes: MMR rounding, being a cool guy. CinderBlock: overall big help <3. mega: ] his SessionStats plugin was a really helpful reference. Others: savior, Martinn, Simple_AOB, HalfwayDead.
// New Credits: Martin for the clean-up, thanks so much! I know it was a mess before, I suck at coding :D
// New New Credits: I don't even know anymore so many people have helped, love this community everyone is so helpful <3

#include "pch.h"
#include "RankViewer.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include "bakkesmod/wrappers/GuiManagerWrapper.h"
#include "utils/parser.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

BAKKESMOD_PLUGIN(RankViewer, "Rank Viewer", "2.1.1", 0)


// Converts information into mmr. Format is directly from game (mode is 11-30, rank is 0-22, div is 0-3). Upper limit is to get the upper part of the range, setting it to false gets the lower part of the mmr range.
int RankViewer::unranker(int mode, int rank, int div, bool upperLimit) {

    string fileName = std::to_string(mode) + ".json";
    
    // Gets the correct json from the folder
    const auto rankJSON = gameWrapper->GetDataFolder() / "RankViewer" / "RankNumbers" / fileName;
    
    string limit;

    if (upperLimit == true) {
        limit = "maxMMR";
    }
    else {
        limit = "minMMR";
    }

    // Stores the json
    std::ifstream file(rankJSON);
    json j = json::parse(file);

    
    // Gets the correct mmr number
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


// Converts rank and div into the usable string that displays on the screen
string GetDivName(int rank, int div) {
    if (rank < 0 || rank > 22) {
        return "ERROR";
    }
    else if (rank == 0 || rank == 22) {
        string rankName = " ";
        return rankName;
    }
    else {
        string divNumbers[] = { "I", "II", "III", "IV" };
        string rankName = "DIV " + divNumbers[div];

        return rankName;
    }


    /*
    if (rank < 0 || rank > 22) {
        return "ERROR";
    }
    else {
        Rank realRank = (Rank)(rank);
        std::string rankName = RankInfoDB[realRank].name;
        string divNumbers[] = { "I", "II", "III", "IV" };

        // Screw you brank for not helping me - Lol ok martinn idk the story behind this XD
        if (rank != Rank::Unranked && rank != Rank::SupersonicLegend)
            rankName += " Div " + divNumbers[div];

        return rankName;
    }*/
}


// Called when loaded in
void RankViewer::onLoad()
{
    // I didn't write this part, no idea how it works. It's for grabbing the mmr from the game
    gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
        cvarManager->executeCommand("togglemenu " + GetMenuName());
        }, 1);

    // Setting for if the plugin is enabled
	cvarManager->registerCvar("rankviewer_enabled", "1", "Enable or Disable the Rank Viewer Plugin", true, true, 0, true, 1, true);

    // Called when game ends
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", bind(&RankViewer::StatsScreen, this, std::placeholders::_1));

    // Called when you leave the stats screen
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&RankViewer::loadMenu, this, std::placeholders::_1));

    // Called when you open the friend tab
    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_MenuStack_TA.ButtonTriggered", bind(&RankViewer::friendScreen, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    friendsOpen.Index = gameWrapper->GetFNameIndexByString("friendsButton");
    friendsClose.Index = gameWrapper->GetFNameIndexByString("closeButton");

    // Puts in the unranked icon as a placeholder for the three images in case something goes wrong later on
    auto testPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / "0.png";
    currentRank = std::make_shared<ImageWrapper>(testPath, false, true);

    testPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / "0.png";
    nextRank = std::make_shared<ImageWrapper>(testPath, false, true);

    testPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / "0.png";
    beforeRank = std::make_shared<ImageWrapper>(testPath, false, true);

    // Screen resolution
    screenSize = gameWrapper->GetScreenSize();
    //safeZone = gameWrapper->GetSafeZoneRatio();
    //uiScale = gameWrapper->GetUIScale();
}


// Called when unloading
void RankViewer::onUnload()
{
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet");
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");
    gameWrapper->UnhookEvent("Function TAGame.GFxData_MenuStack_TA.ButtonTriggered");
    gameWrapper->UnregisterDrawables();
}

// Gets mmr and rank information for displaying
void RankViewer::CheckMMR(int retryCount)
{
    isEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();
    if (!isEnabled) {
        return;
    }

    // The updateMMR section is all from mega's plugin: SessionStats. Please view it here, its great :) https://bakkesplugins.com/plugins/view/39

    ServerWrapper sw = gameWrapper->GetOnlineGame();

    if (sw.IsNull() || !sw.IsOnlineMultiplayer() || gameWrapper->IsInReplay())
        return;

    if (retryCount > 20 || retryCount < 0)
        return;

    if (userPlaylist != 0) {
        gameWrapper->SetTimeout([retryCount, this](GameWrapper* gameWrapper) {
            gotNewMMR = false;
            while (!gotNewMMR) {
                if (1 || (gameWrapper->GetMMRWrapper().IsSynced(uniqueID, userPlaylist) && !gameWrapper->GetMMRWrapper().IsSyncing(uniqueID))) {

                    // Makes sure it is one of the ranked gamemodes to prevent crashes
                    if (!(find(begin(rankedPlaylists), end(rankedPlaylists), userPlaylist) != end(rankedPlaylists))) {
                        return;
                    }

                    // Getting the mmr
                    userMMR = gameWrapper->GetMMRWrapper().GetPlayerMMR(uniqueID, userPlaylist);
                    gotNewMMR = true;

                    MMRWrapper mw = gameWrapper->GetMMRWrapper();

                    // The SkillRank has information about the players rank
                    SkillRank userRank = mw.GetPlayerRank(uniqueID, userPlaylist);

                    // Getting the player rank information into separate variables
                    userDiv = userRank.Division;
                    userTier = userRank.Tier;

                    // Converts the tier and div into the division with the roman numeral (I, II, III, IV)
                    nameCurrent = GetDivName(userTier, userDiv);

                    // Gets and loads the rank icon for your current rank from the RankViewer folder
                    string fileName;
                    fileName = std::to_string(userTier) + ".png";
                    const auto currentPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName;
                    currentRank = std::make_shared<ImageWrapper>(currentPath, false, true);

                    // This all checks for different scenarios where it won't be the default method of displaying
                    if (userTier <= 0) { // --- When still in placement matches -- 
                        // For placement shows from bronze 1 and supersonic legend
                        lowerTier = 1;
                        upperTier = 22;

                        nextLower = unranker(userPlaylist, upperTier, 0, true); // div has to be I (0) since ssl doesn't have divisions
                        nameNext = GetDivName(22, 0);

                        beforeUpper = unranker(userPlaylist, lowerTier, 0, false);
                        nameBefore = GetDivName(0, 0); // This inputs the unranked name since it just won't show the division number
                    }
                    else if (userTier == 1 && userDiv == 0) {
                        // For bronze 1 div 1. It just shows the bronze 1 div 1 lower limit on the bottom and bronze 1 div 2 on top
                        upperTier = userTier;
                        lowerTier = userTier;
                        upperDiv = userDiv + 1;
                        lowerDiv = 0;

                        // Finds the mmr for that div and tier
                        nextLower = unranker(userPlaylist, upperTier, upperDiv, false);
                        nameNext = GetDivName(upperTier, upperDiv);

                        beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, false);
                        nameBefore = GetDivName(lowerTier, lowerDiv);
                    }
                    else if (userTier == 22) {
                        // For ssl. Shows the ssl upper limit on top and gc 3 div 4 on bottom
                        upperTier = userTier;
                        lowerTier = userTier - 1;
                        upperDiv = userDiv;
                        lowerDiv = 3;

                        // Finds the mmr for that div and tier
                        nextLower = unranker(userPlaylist, upperTier, upperDiv, true);
                        nameNext = GetDivName(upperTier, upperDiv);

                        beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, true);
                        nameBefore = GetDivName(lowerTier, lowerDiv);
                    }
                    else {
                        // Finds out what div is above and below you
                        if (userDiv == 0) {
                            upperTier = userTier;
                            lowerTier = userTier - 1;
                            upperDiv = userDiv + 1;
                            lowerDiv = 3;

                        }
                        else if (userDiv == 3) {
                            upperTier = userTier + 1;
                            lowerTier = userTier;
                            upperDiv = 0;
                            lowerDiv = userDiv - 1;
                        }
                        else {
                            upperTier = userTier;
                            lowerTier = userTier;
                            upperDiv = userDiv + 1;
                            lowerDiv = userDiv - 1;
                        }

                        // Finds the mmr for that div and tier
                        nextLower = unranker(userPlaylist, upperTier, upperDiv, false);
                        nameNext = GetDivName(upperTier, upperDiv);

                        beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, true);
                        nameBefore = GetDivName(lowerTier, lowerDiv);
                    }

                    // Gets correct rank icons from folder for before and after ranks
                    fileName = std::to_string(lowerTier) + ".png";
                    const auto beforePath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName;
                    beforeRank = std::make_shared<ImageWrapper>(beforePath, false, true);

                    fileName = std::to_string(upperTier) + ".png";
                    const auto nextPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName;
                    nextRank = std::make_shared<ImageWrapper>(nextPath, false, true);

                    // Lets rank viewer display
                    drawCanvas = true;
                }
                // Failsafe
                if (!gotNewMMR && retryCount > 0) {
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


// Decides if it should render or not
void RankViewer::Render()
{
    // Only displays if the user has the plugin enableded
    isEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();
    if (!isEnabled) {
        return;
    }

    if (isFriendOpen) {
        return;
    }

    // Makes sure you are in the game
    bool inGame = gameWrapper->IsInOnlineGame();
    if (!inGame) {
        drawCanvas = false;
        return;
    }

    if (drawCanvas) {
        RankViewer::RenderImGui();
    }
}


// The actual rendering of imgui
void RankViewer::RenderImGui()
{
    // Percentages for converting to a non-1080p screen
    float xPercent = ((float)screenSize.X / 1920);
    float yPercent = ((float)screenSize.Y / 1080);
    float upperBound = (290);
    float lowerBound = (835);

    // The imgui window allows the quads to show on the screen
    ImVec2 windowPos = ImVec2((1660 * xPercent), 0);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(screenSize.X - (1660 * xPercent) + (10 * xPercent), screenSize.Y + (10 * yPercent)));

    auto gui = gameWrapper->GetGUIManager();
    fontBig = gui.GetFont("PantonBig");

    // Early out if the window is collapsed, as an optimization.
    if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::End();
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Sidebar
    ImVec2 centerPoint = ImVec2(1920, 950);
    drawList->AddQuadFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y)),
                            ImVec2(xPercent * (centerPoint.x - 45), yPercent * (centerPoint.y)),
                            ImVec2(xPercent * (centerPoint.x - 45), yPercent * (centerPoint.y - 820)),
                            ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 820)),
                            IM_COL32_BLACK);
    
    // --- Lower Box ---
    // Displays the box
    centerPoint = ImVec2(1875, 950);
    drawList->AddQuadFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x - 180), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x - 205), yPercent * (centerPoint.y - 45)),
        ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 45)),
        IM_COL32_BLACK);
    drawList->AddTriangleFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 45)),
        ImVec2(xPercent * (centerPoint.x - 35), yPercent * (centerPoint.y - 45)),
        ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 80)),
        IM_COL32_BLACK);
    // Displays fonts
    if (fontBig) {
        float defaultFontSize = 35 * xPercent;
        ImGui::PushFont(fontBig);
        drawList->AddText(fontBig, defaultFontSize, ImVec2(xPercent * (centerPoint.x - 155), yPercent * (centerPoint.y - 40)), ImU32(white), to_string(beforeUpper).c_str());
        ImGui::PopFont();

        defaultFontSize = 25 * xPercent;
        ImGui::PushFont(fontBig);
        drawList->AddText(fontBig, defaultFontSize, ImVec2(xPercent * (centerPoint.x - 85), yPercent * (centerPoint.y - 32)), ImU32(white), "MMR");
        ImGui::PopFont();

        defaultFontSize = 20 * xPercent;
        ImGui::PushFont(fontBig);
        drawList->AddText(fontBig, defaultFontSize, ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 75)), ImU32(white), nameBefore.c_str());
        ImGui::PopFont();
    }
    // Displays rank image
    if (beforeRank->IsLoadedForImGui()) {
        if (auto beforeRankTex = beforeRank->GetImGuiTex()) {
            auto beforeRankRect = beforeRank->GetSizeF();
            ImGui::SetCursorPos(ImVec2(((xPercent * (centerPoint.x - 10)) - windowPos.x), yPercent * (centerPoint.y - 50)));
            ImGui::Image(beforeRankTex, ImVec2(beforeRankRect.X * 0.19f * xPercent, beforeRankRect.Y * 0.19f * yPercent));
        }
    }

    // Upper Box
    // Displays the box
    centerPoint = ImVec2(1875, 175);
    drawList->AddQuadFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x - 180), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x - 205), yPercent * (centerPoint.y - 45)),
        ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 45)),
        IM_COL32_BLACK);
    drawList->AddTriangleFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x - 35), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y + 35)),
        IM_COL32_BLACK);
    // Displays fonts
    if (fontBig) {
        float defaultFontSize = 35 * xPercent;
        ImGui::PushFont(fontBig);
        drawList->AddText(fontBig, defaultFontSize, ImVec2(xPercent * (centerPoint.x - 155), yPercent * (centerPoint.y - 40)), ImU32(white), to_string(nextLower).c_str());
        ImGui::PopFont();

        defaultFontSize = 25 * xPercent;
        ImGui::PushFont(fontBig);
        drawList->AddText(fontBig, defaultFontSize, ImVec2(xPercent * (centerPoint.x - 85), yPercent * (centerPoint.y - 32)), ImU32(white), "MMR");
        ImGui::PopFont();

        defaultFontSize = 20 * xPercent;
        ImGui::PushFont(fontBig);
        drawList->AddText(fontBig, defaultFontSize, ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 40)), ImU32(white), nameNext.c_str());
        ImGui::PopFont();
    }
    // Displays rank image
    if (nextRank->IsLoadedForImGui()) {
        if (auto nextRankTex = nextRank->GetImGuiTex()) {
            auto nextRankRect = nextRank->GetSizeF();
            ImGui::SetCursorPos(ImVec2(((xPercent * (centerPoint.x - 10)) - windowPos.x), yPercent * (centerPoint.y - 25)));
            ImGui::Image(nextRankTex, ImVec2(nextRankRect.X * 0.19f * xPercent, nextRankRect.Y * 0.19f * yPercent));
        }
    }

    // Determines where the middle box should be (y position) based on upper and lower bounds
    yPos = (int)(upperBound + (((nextLower - userMMR) / (nextLower - beforeUpper)) * (lowerBound - upperBound)));
    if (yPos < upperBound) {
        yPos = upperBound;
    }
    else if (yPos > lowerBound) {
        yPos = lowerBound;
    }
    

    // Middle Box
    // Displays the box
    centerPoint = ImVec2(1875, yPos); 
    drawList->AddQuadFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x - 180), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x - 205), yPercent * (centerPoint.y - 45)),
        ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 45)),
        lightBlue);
    drawList->AddQuadFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y + 35)),
        ImVec2(xPercent * (centerPoint.x + 45), yPercent * (centerPoint.y + 35)),
        ImVec2(xPercent * (centerPoint.x + 45), yPercent * (centerPoint.y - 79)),
        ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 79)),
        lightBlue);
    drawList->AddTriangleFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x - 35), yPercent * (centerPoint.y)),
        ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y + 35)),
        lightBlue);
    drawList->AddTriangleFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 45)),
        ImVec2(xPercent * (centerPoint.x - 35), yPercent * (centerPoint.y - 45)),
        ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 80)),
        lightBlue);
    // Displays fonts
    if (fontBig) {
        float defaultFontSize = 35 * xPercent;
        ImGui::PushFont(fontBig);
        drawList->AddText(fontBig, defaultFontSize, ImVec2(xPercent * (centerPoint.x - 155), yPercent * (centerPoint.y - 40)), ImU32(darkBlue), to_string((int)(round(userMMR))).c_str());
        ImGui::PopFont();

        defaultFontSize = 25 * xPercent;
        ImGui::PushFont(fontBig);
        drawList->AddText(fontBig, defaultFontSize, ImVec2(xPercent * (centerPoint.x - 85), yPercent * (centerPoint.y - 32)), ImU32(darkBlue), "MMR");
        ImGui::PopFont();

        defaultFontSize = 20 * xPercent;
        ImGui::PushFont(fontBig);
        drawList->AddText(fontBig, defaultFontSize, ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 65)), ImU32(darkBlue), nameCurrent.c_str());
        ImGui::PopFont();
    }
    // Displays rank image
    if (currentRank->IsLoadedForImGui()) {
        if (auto currentRankTex = currentRank->GetImGuiTex()) {
            auto currentRankRect = currentRank->GetSizeF();
            ImGui::SetCursorPos(ImVec2(((xPercent * (centerPoint.x - 10)) - windowPos.x), yPercent * (centerPoint.y - 40)));
            ImGui::Image(currentRankTex, ImVec2(currentRankRect.X * 0.19f * xPercent, currentRankRect.Y * 0.19f * yPercent));
        }
    }

    ImGui::End();

    if (!isWindowOpen_)
    {
        cvarManager->executeCommand("togglemenu " + GetMenuName());
    }
}


// Name of the menu that is used to toggle the window.
std::string RankViewer::GetMenuName()
{
    return "RankViewer";
}


// Title to give the menu
std::string RankViewer::GetMenuTitle()
{
    return menuTitle_;
}


// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void RankViewer::SetImGuiContext(uintptr_t ctx)
{
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));

    auto gui = gameWrapper->GetGUIManager();

    gui.LoadFont("PantonBig", "Panton-LightCaps.otf", 32);
}


// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool RankViewer::ShouldBlockInput()
{
    return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}


// Return true if window should be interactive
bool RankViewer::IsActiveOverlay()
{
    return false;
}


// Called when window is opened
void RankViewer::OnOpen()
{
    isWindowOpen_ = true;
}


// Called when window is closed
void RankViewer::OnClose()
{
    isWindowOpen_ = false;
}


// Called when the game ends
void RankViewer::StatsScreen(std::string eventName)
{
    isEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();
    if (!isEnabled) {
        return;
    }

    // Getting the playlist and steam/epic id
    MMRWrapper mw = gameWrapper->GetMMRWrapper();

    uniqueID = gameWrapper->GetUniqueID();
    userPlaylist = mw.GetCurrentPlaylist();

    // Gets the screen size in case it changed from the last time
    screenSize = gameWrapper->GetScreenSize();
    //safeZone = gameWrapper->GetSafeZoneRatio();
    //uiScale = gameWrapper->GetUIScale();

    // The friend menu automatically closes when the game finishes
    isFriendOpen = false;

    // Getting the mmr, and after that all of the rank information needed
    if (mw.IsRanked(userPlaylist)) {
        // Makes sure the mmr updates
        CheckMMR(5);
    }
}


// Called when you go back to the main menu
void RankViewer::loadMenu(std::string eventName)
{
    // Removes canvas if you quit the stats screen
    drawCanvas = false;
    isFriendOpen = false;
    
}


// Called when you open or close the friend tab
// Brank love you <3 this all him
void RankViewer::friendScreen(ActorWrapper caller, void* params, const std::string& functionName)
{
    // Temporarily disabling this feature until i get it working better

    /*
    if (params) 
    {
        FName2* menuName = reinterpret_cast<FName2*>(params);

        if (menuName->Index == friendsOpen.Index) {
            if (isFriendOpen) {
                isFriendOpen = false;
            }
            else {
                isFriendOpen = true;
            }

            //cvarManager->log(to_string(isFriendOpen));
        }
        else if (menuName->Index == friendsClose.Index) {
            isFriendOpen = false;
            //cvarManager->log(to_string(isFriendOpen));
        }
    }
    */
    
}


