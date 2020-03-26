// Credits: Bakkes: MMR rounding, being a cool guy. CinderBlock: overall big help <3. mega: while not directly, his SessionStats plugin was a really helpful reference. Others: savior, Martinn, Simple_AOB, HalfwayDead.


// NOTES FOR PLACEMENTS: Have it display your current mmr, plus say that more will show when you finish placements
// Other stuff: Have the canvas part check if the user is in online match. Will have to test if stats screen is considered in online match.


#include "RankViewer.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include <string>

BAKKESMOD_PLUGIN(RankViewer, "Rank Viewer", "1.2.7", 0)

LinearColor colorCurrent;
LinearColor colorBefore;
LinearColor colorNext;
LinearColor colorScheme;

// Converts information into mmr. Format is directly from game (mode is 11-30, rank is 0-19, div is 0-3). Upper limit is to get the upper part of the range, setting it to false gets the lower part of the mmr range.
int unranker(int mode, int rank, int div, bool upperLimit) {

    int realRank, realDiv, realHeight;

    // Changes the rank to descending
    realRank = RANK_Grand_Champion - rank;

    // Since the divisions are descending in the array, we have to flip the index around
    realDiv = DIV_4 - div;

    if (upperLimit) {
        realHeight = 1;
    }
    else {
        realHeight = 0;
    }

    // Converts in-game Rocket League number playlist ids to the correct playlist array
    if (mode == MODE_Ones) {
        return ones[realRank][realDiv][realHeight];
    }
    else if (mode == MODE_Twos) {
        return twos[realRank][realDiv][realHeight];
    }
    else if (mode == MODE_Threes) {
        return threes[realRank][realDiv][realHeight];
    }
    else if (mode == MODE_Solo) {
        return solo[realRank][realDiv][realHeight];
    }
    else if (mode == MODE_Hoops) {
        return hoops[realRank][realDiv][realHeight];
    }
    else if (mode == MODE_Rumble) {
        return rumble[realRank][realDiv][realHeight];
    }
    else if (mode == MODE_Dropshot) {
        return dropshot[realRank][realDiv][realHeight];
    }
    else if (mode == MODE_Snowday) {
        return snowday[realRank][realDiv][realHeight];
    }
	else {
		// if it fails:
		return 0;
	}
}

// Gets the correct colors for each rank
void colorNamer(int rank) {

    if (rank < RANK_Unranked || rank > RANK_Grand_Champion) {
        colorScheme = LinearColor{0,0,0,0};
    }
    else {
        colorScheme = schemes[rank];
    }
}

string rankNamer(int rank, int div) {


	if (rank < RANK_Unranked || rank > RANK_Grand_Champion) {
		return "ERROR";
	}
    else {
		string fullName = rankNames[rank];
        if (rank != RANK_Unranked && rank != RANK_Grand_Champion)
            fullName += divNames[div];

        return fullName;
    }
}

void RankViewer::onLoad()
{
    // Setting for if the plugin is enabled
	cvarManager->registerCvar("rankviewer_enabled", "1", "Enable or Disable the Rank Viewer Plugin", true, true, 0, true, 1, true);

    // Canvas
    gameWrapper->RegisterDrawable(std::bind(&RankViewer::Render, this, std::placeholders::_1));

    // Called when game ends
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", bind(&RankViewer::StatsScreen, this, std::placeholders::_1));

    // Called when you leave the stats screen
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&RankViewer::loadMenu, this, std::placeholders::_1));
}

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
            canvas.SetColor(colorNext.R, colorNext.G, colorNext.B, 255);
            canvas.SetPosition(Vector2{ int(screen.X * .24) , int(screen.Y * .7) });
            canvas.DrawString(nameNext + ": ", 2 * fontSize, 2 * fontSize);

            // 1-3
            canvas.SetColor(233, 238, 240, 255);
            canvas.SetPosition(Vector2{ int(screen.X * .37) , int(screen.Y * .7) });
            canvas.DrawString(std::to_string(nextLower) + nextDiff, 2 * fontSize, 2 * fontSize);
        }
        else {
            // Placement Warning
            canvas.SetColor(colorCurrent.R, colorCurrent.G, colorCurrent.B, 255);
            canvas.SetPosition(Vector2{ int(screen.X * .18) , int(screen.Y * .7) });
            canvas.DrawString("Finish placements for full functionality!", 2 * fontSize, 2 * fontSize);
        }
        
        // 2-1
        canvas.SetColor(233, 238, 240, 255);
        canvas.SetPosition(Vector2{ int(screen.X * .18) , int(screen.Y * .735) });
        canvas.DrawString("Current: ", 2 * fontSize, 2 * fontSize);

        // 2-2
        canvas.SetColor(colorCurrent.R, colorCurrent.G, colorCurrent.B, 255);
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
            canvas.SetColor(colorBefore.R, colorBefore.G, colorBefore.B, 255);
            canvas.SetPosition(Vector2{ int(screen.X * .24) , int(screen.Y * .77) });
            canvas.DrawString(nameBefore + ": ", 2 * fontSize, 2 * fontSize);

            // 3-3
            canvas.SetColor(233, 238, 240, 255);
            canvas.SetPosition(Vector2{ int(screen.X * .37) , int(screen.Y * .77) });
            canvas.DrawString(std::to_string(beforeUpper) + beforeDiff, 2 * fontSize, 2 * fontSize);
        }
    }
}

void RankViewer::StatsScreen(std::string eventName)
{
    isEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();
    if (!isEnabled) {
        return;
    }

    MMRWrapper mw = gameWrapper->GetMMRWrapper();

    mySteamID.ID = gameWrapper->GetSteamID();
    userPlaylist = mw.GetCurrentPlaylist();

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
                if (1 || (gameWrapper->GetMMRWrapper().IsSynced(mySteamID, userPlaylist) && !gameWrapper->GetMMRWrapper().IsSyncing(mySteamID))) {
                    userMMR = gameWrapper->GetMMRWrapper().GetPlayerMMR(mySteamID, userPlaylist);
                    gotNewMMR = true;

                    // This is where my code actually starts lol thanks again mega
                    MMRWrapper mw = gameWrapper->GetMMRWrapper();

                    // The SkillRank has information about the players rank
                    SkillRank userRank = mw.GetPlayerRank(mySteamID, userPlaylist);

                    // Getting the player rank information into seperate variables
                    userDiv = userRank.Division;
                    userTier = userRank.Tier;
                    
                    // Converts the Div and Tier into actual usable names
                    nameCurrent = rankNamer(userTier, userDiv);
                    colorNamer(userTier);
                    colorCurrent = colorScheme;

                    // Checks if the games are placement matches, so that the Next and Before don't show up
                    if (userTier <= RANK_Unranked) {
                        isPlacement = true;
                        drawCanvas = true;
                        return;
                    }
                    else {
                        isPlacement = false;
                    }

                    // Finds out what div is above and below you
                    if (userDiv == DIV_1) {
                        upperTier = userTier;
                        lowerTier = userTier - 1;
                        upperDiv = userDiv + 1;
                        lowerDiv = 3;
                    }
                    else if (userDiv == DIV_4) {
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

                    int diff = 0;

                    // Finds the mmr for that div and tier, and also finds the usable name for display
                    nextLower = unranker(userPlaylist, upperTier, upperDiv, false);
                    diff = nextLower - (int)(round(userMMR));
                    if (diff >= 0) {
                        diff = abs(diff);
                        nextDiff = " (+ " + std::to_string(diff) + ")";
                    }
                    else {
                        diff = abs(diff);
                        nextDiff = " (- " + std::to_string(diff) + ")";
                    }
                    nameNext = rankNamer(upperTier, upperDiv);
                    colorNamer(upperTier);
                    colorNext = colorScheme;

                    beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, true);
                    diff = beforeUpper - (int)(round(userMMR));
                    if (diff <= 0) {
                        diff = abs(diff);
                        beforeDiff = " (- " + std::to_string(diff) + ")";
                    }
                    else {
                        diff = abs(diff);
                        beforeDiff = " (+ " + std::to_string(diff) + ")";
                    }
                    nameBefore = rankNamer(lowerTier, lowerDiv);
                    colorNamer(lowerTier);
                    colorBefore = colorScheme;

                    // Some mmr stats are unknown, so if that happens it will just display nothing
                    if ((nextLower != -1) && (beforeUpper != -1)) {
                        drawCanvas = true;
                    }
                }
                
				if (!gotNewMMR && retryCount > 0) {
                    gameWrapper->SetTimeout([retryCount, this](GameWrapper* gameWrapper) { this->CheckMMR(retryCount - 1); }, 0.5f);
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
}

void RankViewer::onUnload()
{
	//These are automatically handled by bakkesmod
    //gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");
    //gameWrapper->UnregisterDrawables();
}
