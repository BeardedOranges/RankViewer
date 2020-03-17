// Credits: Bakkes: MMR rounding, being a cool guy. CinderBlock: overall big help <3. mega: while not directly, his SessionStats plugin was a really helpful reference. Others: savior, Martinn, Simple_AOB, HalfwayDead.

#include "RankViewer.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include <string>

BAKKESMOD_PLUGIN(RankViewer, "Rank Viewer", "0.1", 0)

int colorCurrent[3];
int colorBefore[3];
int colorNext[3];
int colorScheme[3];

// [Rank descending (Grand Champ, Champ 3, Champ 2, etc)][Divisions descending (IV, III, II, I)][MMR Range ascending (lower limit, upper limit)]
const int ones[20][4][2] = { {{1223, 1325}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1212, 1225}, {1188, 1206}, {1164, 1183}, {1155, 1163}}, {{1132, 1142}, {1108, 1123}, {1085, 1107}, {1074, 1083}}, {{1052, 1060}, {1028, 1048}, {1005, 1025}, {995, 1003}}, {{977, 985}, {958, 976}, {939, 957}, {929, 938}}, {{917, 925}, {898, 916}, {879, 897}, {872, 878}}, {{857, 865}, {838, 856}, {819, 837}, {814, 818}}, {{797, 805}, {778, 796}, {759, 777}, {747, 758}}, {{737, 745}, {718, 736}, {699, 717}, {692, 698}}, {{677, 684}, {658, 676}, {639, 657}, {633, 638}}, {{617, 624}, {598, 616}, {579, 597}, {571, 578}}, {{557, 564}, {538, 556}, {519, 537}, {513, 518}}, {{497, 505}, {478, 496}, {459, 477}, {453, 458}}, {{437, 445}, {418, 436}, {399, 417}, {392, 398}}, {{377, 384}, {358, 376}, {339, 357}, {332, 338}}, {{317, 322}, {298, 316}, {279, 297}, {273, 278}}, {{257, 265}, {238, 256}, {219, 237}, {212, 218}}, {{197, 200}, {178, 196}, {159, 177}, {148, 158}}, {{142, 153}, {128, 140}, {114, 127}, {0, 113}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int twos[20][4][2] = { {{1500, 1708}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1482, 1500}, {1448, 1466}, {1415, 1436}, {1395, 1413}}, {{1367, 1380}, {1338, 1365}, {1312, 1337}, {1295, 1308}}, {{1267, 1280}, {1238, 1265}, {1210, 1237}, {1195, 1208}}, {{1167, 1180}, {1138, 1165}, {1110, 1137}, {1094, 1108}}, {{1072, 1080}, {1048, 1071}, {1024, 1047}, {1013, 1023}}, {{992, 1000}, {968, 991}, {944, 967}, {933, 943}}, {{912, 920}, {888, 911}, {864, 887}, {852, 863}}, {{832, 840}, {808, 831}, {784, 807}, {773, 783}}, {{752, 760}, {728, 751}, {704, 727}, {693, 703}}, {{672, 680}, {648, 671}, {624, 647}, {612, 623}}, {{597, 605}, {578, 596}, {559, 577}, {549, 558}}, {{537, 545}, {518, 536}, {499, 517}, {489, 498}}, {{477, 485}, {458, 476}, {439, 457}, {429, 438}}, {{417, 424}, {398, 416}, {379, 397}, {368, 378}}, {{357, 365}, {338, 356}, {319, 337}, {310, 318}}, {{297, 305}, {278, 296}, {259, 277}, {250, 258}}, {{237, 242}, {218, 236}, {199, 217}, {190, 198}}, {{172, 180}, {148, 171}, {125, 147}, {0, 123}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int threes[20][4][2] = { {{1501, 1710}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1482, 1500}, {1448, 1466}, {1415, 1434}, {1395, 1413}}, {{1367, 1380}, {1338, 1365}, {1310, 1337}, {1295, 1308}}, {{1267, 1280}, {1238, 1265}, {1210, 1237}, {1195, 1208}}, {{1167, 1180}, {1138, 1165}, {1110, 1137}, {1095, 1108}}, {{1072, 1080}, {1048, 1071}, {1024, 1047}, {1013, 1023}}, {{992, 1000}, {968, 991}, {944, 967}, {934, 943}}, {{912, 920}, {888, 911}, {864, 887}, {852, 863}}, {{832, 840}, {808, 831}, {784, 807}, {772, 783}}, {{752, 761}, {728, 751}, {704, 727}, {695, 703}}, {{672, 681}, {648, 671}, {624, 647}, {613, 623}}, {{597, 605}, {578, 596}, {559, 577}, {548, 558}}, {{537, 545}, {518, 536}, {499, 517}, {490, 498}}, {{477, 484}, {458, 476}, {439, 457}, {430, 438}}, {{417, 425}, {398, 416}, {379, 397}, {370, 378}}, {{357, 365}, {338, 356}, {319, 337}, {310, 318}}, {{297, 302}, {278, 296}, {259, 277}, {250, 258}}, {{237, 243}, {218, 236}, {199, 217}, {189, 198}}, {{172, 180}, {155, 170}, {125, 147}, {0, 123}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int solo[20][4][2] = { {{1287, 1319}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1272, 1294}, {1248, 1264}, {1224, 1237}, {1211, 1223}}, {{1192, 1201}, {1168, 1188}, {1145, 1165}, {1133, 1143}}, {{1112, 1123}, {1088, 1110}, {1064, 1087}, {1055, 1063}}, {{1032, 1041}, {1008, 1030}, {984, 1007}, {973, 983}}, {{952, 962}, {928, 951}, {904, 927}, {895, 903}}, {{872, 881}, {848, 871}, {824, 847}, {815, 823}}, {{797, 802}, {778, 796}, {759, 777}, {750, 758}}, {{737, 744}, {718, 736}, {699, 717}, {689, 698}}, {{677, 683}, {658, 676}, {639, 657}, {633, 638}}, {{617, 625}, {598, 616}, {579, 597}, {570, 578}}, {{557, 563}, {538, 556}, {519, 537}, {511, 518}}, {{497, 506}, {478, 496}, {459, 477}, {453, 458}}, {{437, 445}, {418, 436}, {399, 417}, {392, 398}}, {{377, 386}, {358, 376}, {339, 357}, {330, 338}}, {{317, 323}, {298, 316}, {279, 297}, {270, 278}}, {{257, 270}, {238, 255}, {220, 237}, {211, 218}}, {{197, 210}, {178, 195}, {160, 177}, {151, 158}}, {{142, 154}, {133, 140}, {115, 127}, {0, 113}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int hoops[20][4][2] = { {{1242, 1322}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1232, 1242}, {1208, 1225}, {1185, 1207}, {1167, 1183}}, {{1152, 1160}, {1128, 1150}, {1105, 1127}, {1089, 1103}}, {{1072, 1081}, {1048, 1071}, {1024, 1047}, {1015, 1023}}, {{992, 1000}, {968, 991}, {944, 967}, {930, 943}}, {{917, 923}, {898, 916}, {879, 897}, {868, 878}}, {{857, 862}, {838, 856}, {819, 837}, {813, 818}}, {{797, 803}, {778, 796}, {759, 777}, {747, 758}}, {{737, 743}, {718, 736}, {699,717}, {689, 698}}, {{677, 681}, {658, 676}, {639, 657}, {633, 638}}, {{617, 622}, {598, 616}, {579, 597}, {572, 578}}, {{557, 563}, {538, 556}, {519, 537}, {510, 518}}, {{497, 504}, {478, 496}, {459, 477}, {451, 458}}, {{437, 447}, {418, 436}, {399, 417}, {383, 398}}, {{377, 380}, {358, 376}, {339, 357}, {321, 338}}, {{317, 327}, {301, 316}, {281, 297}, {266, 277}}, {{258, 274}, {238, 256}, {219, 228}, {211, 213}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int rumble[20][4][2] = { {{1241, 1330}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1232, 1242}, {1208, 1227}, {1185, 1207}, {1170, 1183}}, {{1152, 1160}, {1128, 1150}, {1105, 1127}, {1093, 1103}}, {{1072, 1080}, {1048, 1071}, {1024, 1047}, {1015, 1023}}, {{992, 1000}, {968, 991}, {944, 967}, {933, 943}}, {{917, 923}, {898, 916}, {879, 897}, {870, 878}}, {{857, 864}, {838, 856}, {819, 837}, {809, 818}}, {{797, 803}, {778, 796}, {759, 777}, {748, 758}}, {{737, 744}, {718, 736}, {699,717}, {691, 698}}, {{677, 685}, {658, 676}, {639, 657}, {630, 638}}, {{617, 623}, {598, 616}, {579, 597}, {571, 578}}, {{557, 560}, {538, 556}, {519, 537}, {511, 518}}, {{497, 504}, {478, 496}, {459, 477}, {449, 458}}, {{437, 441}, {418, 436}, {399, 417}, {388, 398}}, {{377, 385}, {358, 376}, {340, 357}, {328, 338}}, {{317, 325}, {298, 315}, {280, 297}, {266, 278}}, {{257, 267}, {238, 255}, {225, 237}, {206, 218}}, {{197, 200}, {178, 196}, {164, 177}, {153, 157}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int dropshot[20][4][2] = { {{1244, 1298}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1232, 1250}, {1208, 1230}, {1185, 1204}, {1166, 1183}}, {{1152, 1162}, {1128, 1149}, {1105, 1124}, {1094, 1103}}, {{1072, 1080}, {1048, 1070}, {1024, 1047}, {1015, 1023}}, {{992, 1002}, {968, 991}, {944, 967}, {935, 943}}, {{917, 925}, {898, 916}, {879, 897}, {873, 878}}, {{857, 862}, {838, 856}, {819, 837}, {811, 818}}, {{797, 805}, {778, 796}, {759, 777}, {749, 758}}, {{737, 742}, {718, 736}, {699,717}, {692, 698}}, {{677, 682}, {658, 676}, {639, 657}, {630, 638}}, {{617, 621}, {598, 616}, {579, 597}, {573, 578}}, {{557, 562}, {538, 556}, {519, 537}, {512, 518}}, {{497, 502}, {478, 496}, {459, 477}, {454, 458}}, {{437, 443}, {418, 436}, {399, 417}, {385, 398}}, {{377, 388}, {358, 376}, {339, 357}, {323, 338}}, {{317, 329}, {299, 316}, {279, 297}, {268, 278}}, {{257, 272}, {238, 255}, {220, 237}, {203, 218}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int snowday[20][4][2] = { {{1241, 1312}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1232, 1240}, {1208, 1227}, {1185, 1205}, {1165, 1183}}, {{1152, 1163}, {1128, 1150}, {1105, 1127}, {1091, 1103}}, {{1072, 1080}, {1048, 1071}, {1024, 1047}, {1015, 1023}}, {{992, 1000}, {968, 991}, {944, 967}, {935, 943}}, {{917, 926}, {898, 916}, {879, 897}, {869, 878}}, {{857, 863}, {838, 856}, {819, 837}, {811, 818}}, {{797, 805}, {778, 796}, {759, 777}, {747, 758}}, {{737, 743}, {718, 736}, {699,717}, {688, 698}}, {{677, 685}, {658, 676}, {639, 657}, {634, 638}}, {{617, 622}, {598, 616}, {579, 597}, {566, 578}}, {{557, 573}, {538, 556}, {519, 537}, {505, 518}}, {{497, 508}, {478, 496}, {459, 477}, {446, 458}}, {{437, 447}, {418, 436}, {400, 417}, {384, 398}}, {{377, 388}, {358, 375}, {341, 357}, {327, 338}}, {{317, 332}, {301, 314}, {282, 297}, {266, 277}}, {{257, 263}, {238, 251}, {219, 235}, {211, 217}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };

// Converts information into mmr. Format is directly from game (mode is 11-30, rank is 0-19, div is 0-3). Upper limit is to get the upper part of the range, setting it to false gets the lower part of the mmr range.
int unranker(int mode, int rank, int div, bool upperLimit) {

    int realRank, realDiv, realHeight;

    // Changes the rank to descending 
    int rankIndex[] = { 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    realRank = rankIndex[rank];

    // Since the divisions are descending in the array, we have to flip the index around
    int divIndex[] = { 3, 2, 1, 0 };
    realDiv = divIndex[div];

    if (upperLimit) {
        realHeight = 1;
    }
    else {
        realHeight = 0;
    }

    // Converts in-game Rocket League number playlist ids to the correct playlist array
    if (mode == 10) {
        return ones[realRank][realDiv][realHeight];
    }
    else if (mode == 11) {
        return twos[realRank][realDiv][realHeight];
    }
    else if (mode == 13) {
        return threes[realRank][realDiv][realHeight];
    }
    else if (mode == 12) {
        return solo[realRank][realDiv][realHeight];
    }
    else if (mode == 27) {
        return hoops[realRank][realDiv][realHeight];
    }
    else if (mode == 28) {
        return rumble[realRank][realDiv][realHeight];
    }
    else if (mode == 29) {
        return dropshot[realRank][realDiv][realHeight];
    }
    else if (mode == 30) {
        return snowday[realRank][realDiv][realHeight];
    }

}

void colorNamer(int rank) {
    int schemes[20][3] =
    {
        { 133, 133, 133 }, // Unranked | 0
        { 227, 151, 68 },  // Bronze 1 | 1
        { 227, 151, 68}, // Bronze 2 | 2
        { 227, 151, 68}, // Bronze 3 | 3
        { 133, 133, 133}, // Silver 1 | 4
        { 133, 133, 133}, // Silver 2 | 5
        { 133, 133, 133}, // Silver 3 | 6
        { 202, 149, 31}, // Gold 1 | 7
        { 202, 149, 31}, // Gold 2 | 8
        { 202, 149, 31}, // Gold 3 | 9
        { 50, 204, 252}, // Platinum 1 | 10
        { 50, 204, 252}, // Platinum 2 | 11
        { 50, 204, 252}, // Platinum 3 | 12
        { 2, 188, 255}, // Diamond 1 | 13
        { 2, 188, 255}, // Diamond 2 | 14
        { 2, 188, 255}, // Diamond 3 | 15
        { 202, 137, 255 }, // Champion 1 | 16
        { 202, 137, 255 }, // Champion 2 | 17
        { 202, 137, 255 }, // Champion 3 | 18
        { 244, 56, 236 } // Grand Champion | 19
    };

    colorScheme[0] = schemes[rank][0];
    colorScheme[1] = schemes[rank][1];
    colorScheme[2] = schemes[rank][2];
}

string rankNamer(int rank, int div) {
    string fullName = "";

    string rankNames[] =
    {
        "Unranked",
        "Bronze 1",
        "Bronze 2",
        "Bronze 3",
        "Silver 1",
        "Silver 2",
        "Silver 3",
        "Gold 1",
        "Gold 2",
        "Gold 3",
        "Platinum 1",
        "Platinum 2",
        "Platinum 3",
        "Diamond 1",
        "Diamond 2",
        "Diamond 3",
        "Champion 1",
        "Champion 2",
        "Champion 3",
        "Grand Champion"
    };

    string divNames[] =
    {
        " Div 1",
        " Div 2",
        " Div 3",
        " Div 4"
    };

    fullName += rankNames[rank];

    if (fullName.find("Grand Champion") == string::npos)
        fullName += divNames[div];

    return fullName;
}

void RankViewer::onLoad()
{
    // Setting for if the plugin is enabled
    cvarManager->registerCvar("rankviewer_enabled", "1", "Enable or Disable the Rank Viewer Plugin", true, true, 0, true, 1, true);

    gameWrapper->RegisterDrawable(std::bind(&RankViewer::Render, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", bind(&RankViewer::StatsScreen, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&RankViewer::loadMenu, this, std::placeholders::_1));
}

void RankViewer::Render(CanvasWrapper canvas)
{
    isEnabled = cvarManager->getCvar("rankviewer_enabled").getBoolValue();
    if (!isEnabled) {
        return;
    }

    if (drawCanvas) {

        Vector2 screen = canvas.GetSize();

        float fontSize = (float)screen.X / (float)1920;

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

void RankViewer::loadMenu(std::string eventName)
{
    // Removes canvas if you quit the stats screen
    drawCanvas = false;
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
                    MMRWrapper nw = gameWrapper->GetMMRWrapper();

                    // The SkillRank has information about the players rank
                    SkillRank userRank = nw.GetPlayerRank(mySteamID, userPlaylist);

                    // Getting the player rank information into seperate variables
                    userDiv = userRank.Division;
                    userTier = userRank.Tier;

                    // Converts the Div and Tier into actual usable names
                    nameCurrent = rankNamer(userTier, userDiv);
                    colorNamer(userTier);
                    memcpy(colorCurrent, colorScheme, sizeof(colorScheme));

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
                    memcpy(colorNext, colorScheme, sizeof(colorScheme));

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
                    memcpy(colorBefore, colorScheme, sizeof(colorScheme));

                    // Some mmr stats are unknown, so if that happens it will just display nothing
                    if ((nextLower != -1) && (beforeUpper != -1)) {
                        drawCanvas = true;
                    }
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

void RankViewer::onUnload()
{
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");
    gameWrapper->UnregisterDrawables();
}