// Credits: Bakkes: MMR rounding, being a cool guy. CinderBlock: overall big help <3. mega: while not directly, his SessionStats plugin was a really helpful reference. Others: savior, Martinn, Simple_AOB, HalfwayDead.
// New Credits: Martin for the clean-up, thanks so much! I know it was a mess before, I suck at coding :D
#include "pch.h"
#include "RankViewer.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include "bakkesmod/wrappers/GuiManagerWrapper.h"
#include "utils/parser.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;


BAKKESMOD_PLUGIN(RankViewer, "Rank Viewer", "2.0", 0)


// Converts information into mmr. Format is directly from game (mode is 11-30, rank is 0-22, div is 0-3). Upper limit is to get the upper part of the range, setting it to false gets the lower part of the mmr range.
int RankViewer::unranker(int mode, int rank, int div, bool upperLimit) {


    string fileName = std::to_string(mode) + ".json";
    
    const auto rankJSON = gameWrapper->GetDataFolder() / "RankViewer" / "RankNumbers" / fileName;
    
    string limit;

    if (upperLimit == true) {
        limit = "maxMMR";
    }
    else {
        limit = "minMMR";
    }

    std::ifstream file(rankJSON);
    json j = json::parse(file);

    /*
    int mmrNum = -1;

    for (int id = 0; id <= j["data"]["data"].size(); id++) {
        if (j["data"]["data"][id]["tier"] == rank && j["data"]["data"][id]["division"] == div) {
            mmrNum = j["data"]["data"][id][limit];
        }
    }
    

    return mmrNum; // return here
    */

    return j["data"]["data"][((rank - 1) * 4) + (div + 1)][limit];
    /*
    int realRank, realDiv;

    // Changes the rank to descending 
    int rankIndex[] = {22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    realRank = rankIndex[rank];

    // Since the divisions are descending in the array, we have to flip the index around
    int divIndex[] = { 3, 2, 1, 0 };
    realDiv = divIndex[div];

    auto divData = GetDivisionData((Playlists)mode, (Rank)realRank, realDiv);
    return upperLimit ? divData.higher : divData.lower;
    */
}


//// Gets the correct colors for each rank
//void SetRankColor(int rank) {
//    Rank realRank = (Rank)(rank);
//    if (realRank < Rank::Unranked || realRank > Rank::GrandChamp)
//    {
//        colorScheme[0] = 0;
//        colorScheme[1] = 0;
//        colorScheme[2] = 0;
//        
//    }
//    else {
//        auto color = RankInfoDB[realRank].color;
//        colorScheme[0] = color.r;
//        colorScheme[1] = color.g;
//        colorScheme[2] = color.b;
//    }
//}

string GetRankName(int rank, int div) {
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

void RankViewer::onLoad()
{


    gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
        cvarManager->executeCommand("togglemenu " + GetMenuName());
        }, 1);

    // Setting for if the plugin is enabled
	cvarManager->registerCvar("rankviewer_enabled", "1", "Enable or Disable the Rank Viewer Plugin", true, true, 0, true, 1, true);

    // Canvas for screen size
    //gameWrapper->RegisterDrawable(std::bind(&RankViewer::RenderCanvas, this, std::placeholders::_1));

    // Called when game ends
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", bind(&RankViewer::StatsScreen, this, std::placeholders::_1));

    // Called when you leave the stats screen
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&RankViewer::loadMenu, this, std::placeholders::_1));

    friendsOpen.Index = gameWrapper->GetFNameIndexByString("friendsButton");
    friendsClose.Index = gameWrapper->GetFNameIndexByString("closeButton");
    // Called when you open the friend tab
    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_MenuStack_TA.ButtonTriggered", bind(&RankViewer::friendScreen, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // Debug command
    //cvarManager->registerNotifier("debug_mmr", [this](std::vector<std::string> args) {DebugGetDivisionData(args); }, "testing", PERMISSION_MENU);

    auto testPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / "0.png";
    currentRank = std::make_shared<ImageWrapper>(testPath, false, true);

    testPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / "0.png";
    nextRank = std::make_shared<ImageWrapper>(testPath, false, true);

    testPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / "0.png";
    beforeRank = std::make_shared<ImageWrapper>(testPath, false, true);

    screenSize = gameWrapper->GetScreenSize();
    //safeZone = gameWrapper->GetSafeZoneRatio();
    //uiScale = gameWrapper->GetUIScale();
}


void RankViewer::Render()
{
    // Only displays if the user has the plugin enableds
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
        return; //undo this comment on release
    }

    if (drawCanvas) {
        RankViewer::RenderImGui();
    }
}

void RankViewer::RenderImGui()
{
    float xPercent = ((float)screenSize.X / 1920);
    float yPercent = ((float)screenSize.Y / 1080);
    float upperBound = (290);
    float lowerBound = (835);

    ImVec2 windowPos = ImVec2((1660 * xPercent), 0);

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(screenSize.X - (1660 * xPercent) + (10 * xPercent), screenSize.Y + (10 * yPercent)));

    auto gui = gameWrapper->GetGUIManager();
    fontBig = gui.GetFont("PantonBig");

    if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    /*
    nextLower = 1342;
    beforeUpper = 1335;
    userMMR = 1336;
    */

    //ImGui::ShowDemoWindow();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    

    
    

    // Sidebar
    ImVec2 centerPoint = ImVec2(1920, 950);
    drawList->AddQuadFilled(ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y)),
                            ImVec2(xPercent * (centerPoint.x - 45), yPercent * (centerPoint.y)),
                            ImVec2(xPercent * (centerPoint.x - 45), yPercent * (centerPoint.y - 820)),
                            ImVec2(xPercent * (centerPoint.x), yPercent * (centerPoint.y - 820)),
                            IM_COL32_BLACK);

    
    // Lower Box
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

    // Displays image
    if (beforeRank->IsLoadedForImGui()) {
        if (auto beforeRankTex = beforeRank->GetImGuiTex()) {
            auto beforeRankRect = beforeRank->GetSizeF();
            ImGui::SetCursorPos(ImVec2(((xPercent * (centerPoint.x - 10)) - windowPos.x), yPercent * (centerPoint.y - 50)));
            ImGui::Image(beforeRankTex, ImVec2(beforeRankRect.X * 0.19f * xPercent, beforeRankRect.Y * 0.19f * yPercent));
        }
    }

    // Upper Box
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

    /*
    std::tuple<int, ImFont*> newFont = gui.LoadFont("PantonBig", "Panton-LightCaps.otf", 30);
    std::tuple<int, ImFont*> newFont3 = gui.LoadFont("PantonSmall", "Panton-LightCaps.otf", 1);

    if (std::get<0>(newFont) == 0) {
        cvarManager->log("Failed to load the font!");
    }
    else if (std::get<0>(newFont) == 1) {
        cvarManager->log("The font will be loaded");
    }
    else if (std::get<0>(newFont) == 2 && std::get<1>(newFont)) {
        fontBig = std::get<1>(newFont);
        fontSmall = std::get<1>(newFont3);
    }
    */
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

/*
void RankViewer::Render(CanvasWrapper canvas)
{
    // Only displays if the user has the plugin enableds
    isEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();
    if (!isEnabled) {
        return;
    }

    // Makes sure you are in the game
    bool inGame = gameWrapper->IsInOnlineGame();
    if (!inGame) {
        drawCanvas = false;
        return;
    }

    if (drawCanvas) {
        
        Vector2 screen = canvas.GetSize();

        float fontSize = (float)screen.X / (float)1920;

        if (!isPlacement) {
            // 1-1
            canvas.SetColor(233, 238, 240, 255);
            canvas.SetPosition(Vector2{ int(screen.X * .2) , int(screen.Y * .7) });
            canvas.DrawString("Next: ", 2 * fontSize, 2 * fontSize);

            // 1-2
            canvas.SetColor(colorNext[0], colorNext[1], colorNext[2], 255);
            canvas.SetPosition(Vector2{ int(screen.X * .24) , int(screen.Y * .7) });
            canvas.DrawString(nameNext + ": ", 2 * fontSize, 2 * fontSize);

            // 1-3
            canvas.SetColor(233, 238, 240, 255);
            canvas.SetPosition(Vector2{ int(screen.X * .37) , int(screen.Y * .7) });
            canvas.DrawString(std::to_string(nextLower) + nextDiff, 2 * fontSize, 2 * fontSize);
        }
        else {
            // Placement Warning
            canvas.SetColor(colorCurrent[0], colorCurrent[1], colorCurrent[2], 255);
            canvas.SetPosition(Vector2{ int(screen.X * .18) , int(screen.Y * .7) });
            canvas.DrawString("Finish placements for full functionality!", 2 * fontSize, 2 * fontSize);
        }
        
        // 2-1
        canvas.SetColor(233, 238, 240, 255);
        canvas.SetPosition(Vector2{ int(screen.X * .18) , int(screen.Y * .735) });
        canvas.DrawString("Current: ", 2 * fontSize, 2 * fontSize);

        // 2-2
        canvas.SetColor(colorCurrent[0], colorCurrent[1], colorCurrent[2], 255);
        canvas.SetPosition(Vector2{ int(screen.X * .24) , int(screen.Y * .735) });
        canvas.DrawString(nameCurrent + ": ", 2 * fontSize, 2 * fontSize);

        // 2-3
        canvas.SetColor(233, 238, 240, 255);
        canvas.SetPosition(Vector2{ int(screen.X * .37) , int(screen.Y * .735) });
        canvas.DrawString(std::to_string((int)(round(userMMR))), 2 * fontSize, 2 * fontSize);

        if (!isPlacement) {
            // 3-1
            canvas.SetColor(233, 238, 240, 255);
            canvas.SetPosition(Vector2{ int(screen.X * .175) , int(screen.Y * .77) });
            canvas.DrawString("Previous: ", 2 * fontSize, 2 * fontSize);

            // 3-2
            canvas.SetColor(colorBefore[0], colorBefore[1], colorBefore[2], 255);
            canvas.SetPosition(Vector2{ int(screen.X * .24) , int(screen.Y * .77) });
            canvas.DrawString(nameBefore + ": ", 2 * fontSize, 2 * fontSize);

            // 3-3
            canvas.SetColor(233, 238, 240, 255);
            canvas.SetPosition(Vector2{ int(screen.X * .37) , int(screen.Y * .77) });
            canvas.DrawString(std::to_string(beforeUpper) + beforeDiff, 2 * fontSize, 2 * fontSize);
        }
    }
}
*/

void RankViewer::StatsScreen(std::string eventName)
{
    isEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();
    if (!isEnabled) {
        return;
    }

    MMRWrapper mw = gameWrapper->GetMMRWrapper();

    uniqueID = gameWrapper->GetUniqueID();
    userPlaylist = mw.GetCurrentPlaylist();

    // Maybe delete this idk
    screenSize = gameWrapper->GetScreenSize();
    //safeZone = gameWrapper->GetSafeZoneRatio();
    //uiScale = gameWrapper->GetUIScale();

    isFriendOpen = false;

    if (mw.IsRanked(userPlaylist)) {

        // Makes sure the mmr updates
        CheckMMR(5);
    }
}

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
                    userMMR = gameWrapper->GetMMRWrapper().GetPlayerMMR(uniqueID, userPlaylist);
                    gotNewMMR = true;

                    // This is where my code actually starts lol thanks again mega
                    MMRWrapper mw = gameWrapper->GetMMRWrapper();

                    // The SkillRank has information about the players rank
                    SkillRank userRank = mw.GetPlayerRank(uniqueID, userPlaylist);

                    // Getting the player rank information into seperate variables
                    userDiv = userRank.Division;
                    userTier = userRank.Tier;
                    
                    // Converts the Div and Tier into actual usable names
                    nameCurrent = GetRankName(userTier, userDiv);
                    //SetRankColor(userTier);
                    //memcpy(colorCurrent, colorScheme, sizeof(colorScheme));

                    string fileName;

                    fileName = std::to_string(userTier) + ".png";
                    const auto currentPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName;
                    currentRank = std::make_shared<ImageWrapper>(currentPath, false, true);

                    //cvarManager->log(currentPath.c_str());

                    if (userTier <= 0) {
                        isPlacement = true;

                        lowerTier = 1;
                        upperTier = 22;

                        nextLower = unranker(userPlaylist, upperTier, 0, true);
                        nameNext = GetRankName(22, 0);

                        beforeUpper = unranker(userPlaylist, lowerTier, 0, false);
                        nameBefore = GetRankName(0, 0); // technically this is unranked but i just don't want the div to show
                    }
                    else if (userTier == 1 && userDiv == 0) {
                        upperTier = userTier;
                        lowerTier = userTier;
                        upperDiv = userDiv + 1;
                        lowerDiv = 0;

                        // Finds the mmr for that div and tier
                        nextLower = unranker(userPlaylist, upperTier, upperDiv, false);
                        nameNext = GetRankName(upperTier, upperDiv);

                        beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, false);
                        nameBefore = GetRankName(lowerTier, lowerDiv);
                    }
                    else if (userTier == 22) {
                        upperTier = userTier;
                        lowerTier = userTier - 1;
                        upperDiv = userDiv;
                        lowerDiv = 3;

                        // Finds the mmr for that div and tier
                        nextLower = unranker(userPlaylist, upperTier, upperDiv, true);
                        nameNext = GetRankName(upperTier, upperDiv);

                        beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, true);
                        nameBefore = GetRankName(lowerTier, lowerDiv);
                    }
                    else {
                        isPlacement = false;
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
                        nameNext = GetRankName(upperTier, upperDiv);

                        beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, true);
                        nameBefore = GetRankName(lowerTier, lowerDiv);
                    }

                    // Gets correct rank icons from folder  HEYEYYEYYEY DON't forget that the currentRank needs to be before the placement checl
                    fileName = std::to_string(lowerTier) + ".png";
                    const auto beforePath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName;
                    beforeRank = std::make_shared<ImageWrapper>(beforePath, false, true);

                    fileName = std::to_string(upperTier) + ".png";
                    const auto nextPath = gameWrapper->GetDataFolder() / "RankViewer" / "RankIcons" / fileName;
                    nextRank = std::make_shared<ImageWrapper>(nextPath, false, true);
                    
                    drawCanvas = true;
                }
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


void RankViewer::loadMenu(std::string eventName)
{
    // Removes canvas if you quit the stats screen
    drawCanvas = false;
    isFriendOpen = false;
    
}

// Brank love you <3 this all him
void RankViewer::friendScreen(ActorWrapper caller, void* params, const std::string& functionName)
{
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

            cvarManager->log(to_string(isFriendOpen));
        }
        else if (menuName->Index == friendsClose.Index) {
            isFriendOpen = false;
            cvarManager->log(to_string(isFriendOpen));
        }
    }

    
}

void RankViewer::onUnload()
{
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");
    gameWrapper->UnregisterDrawables();
}

/*
DivisionData RankViewer::GetDivisionData(Playlists mode, Rank rank, int div)
{
    auto playlistSearch = playlistMMRDatabase.find(mode);
    if (playlistSearch != playlistMMRDatabase.end())
    {
        auto playlist = playlistSearch->second;
        if (playlist.tiers.size() > rank)
        {
            auto tier = playlist.tiers[rank];
            if (tier.divisions.size() > div)
            {
                return tier.divisions[div];
            }
        }
    }
    return DivisionData();
}

void RankViewer::DebugGetDivisionData(std::vector<std::string> args)
{
    if (args.size() != 4)
    {
        cvarManager->log("Usage:" + args[0] + " mode rank div");
        return;
    }

    Playlists mode =  (Playlists) get_safe_int(args[1]);
    Rank rank = (Rank) get_safe_int(args[2]);
    int div = get_safe_int(args[3]);

    auto divData = GetDivisionData(mode, rank, div);
    cvarManager->log("Lower:" + to_string(divData.lower) + " Higher: " + to_string(divData.higher));
}
*/

