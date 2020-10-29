// Credits: Bakkes: MMR rounding, being a cool guy. CinderBlock: overall big help <3. mega: while not directly, his SessionStats plugin was a really helpful reference. Others: savior, Martinn, Simple_AOB, HalfwayDead.
#include "pch.h"
#include "RankViewer.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include "String.h"
#include "RankEnums.h"
#include "PlaylistData.h"




BAKKESMOD_PLUGIN(RankViewer, "Rank Viewer", "1.2", 0)

int colorCurrent[3];
int colorBefore[3];
int colorNext[3];
int colorScheme[3];

// [Rank descending (Grand Champ, Champ 3, Champ 2, etc)][Divisions descending (IV, III, II, I)][MMR Range ascending (lower limit, upper limit)]
const int ones[23][4][2]       = { {{1345, 1407}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1337, 1352}, {1318, 1335}, {1300, 1316}, {1287, 1298}}, {{1277, 1291}, {1258, 1275}, {1240, 1255}, {1226, 1238}}, {{1217, 1225}, {1202, 1214}, {1180, 1187}, {1175, 1178}}, {{1157, 1165}, {1140, 1155}, {1120, 1136}, {1105, 1118}}, {{1097, 1106}, {1079, 1095}, {1060, 1074}, {1046, 1058}}, {{1037, 1045}, {1018, 1035}, {1014, 1000}, {987, 998}}, {{977, 986}, {958, 975}, {940, 957}, {926, 938}}, {{917, 926}, {898, 916}, {879, 897}, {867, 878}}, {{857, 866}, {838, 856}, {819, 837}, {807, 818}}, {{797, 805}, {778, 796}, {759, 777}, {747, 758}}, {{737, 745}, {718, 736}, {699, 717}, {697, 698}}, {{677, 686}, {658, 676}, {639, 657}, {687, 698}}, {{617, 626}, {598, 616}, {579, 597}, {573, 578}}, {{557, 565}, {538, 556}, {519, 537}, {513, 518}}, {{497, 501}, {478, 496}, {459, 477}, {455, 458}}, {{437, 453}, {418, 436}, {399, 417}, {384, 398}}, {{377, 394}, {358, 376}, {339, 357}, {321, 338}}, {{317, 327}, {298, 316}, {279, 297}, {265, 278}}, {{257, 275}, {238, 256}, {220, 237}, {202, 218}}, {{197, 207}, {179, 196}, {162, 174}, {153, 154}}, {{142, 153}, {132, 139}, {117, 127}, {0, 113}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int twos[23][4][2]       = { {{1863, 1953}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1843, 1861}, {1808, 1835}, {1775, 1805}, {1752, 1773}}, {{1722, 1740}, {1688, 1715}, {1654, 1674}, {1635, 1653}}, {{1602, 1619}, {1568, 1588}, {1535, 1566}, {1515, 1533}}, {{1487, 1504}, {1462, 1485}, {1432, 1457}, {1413, 1428}}, {{1387, 1399}, {1363, 1385}, {1332, 1357}, {1313, 1328}}, {{1287, 1300}, {1262, 1285}, {1232, 1256}, {1214, 1228}}, {{1187, 1200}, {1158, 1185}, {1130, 1157}, {1115, 1228}}, {{1087, 1100}, {1058, 1085}, {1032, 1056}, {1014, 1028}}, {{992, 1000}, {968, 987}, {945, 967}, {933, 943}}, {{912, 920}, {888, 910}, {865, 887}, {854, 863}}, {{832, 840}, {808, 831}, {784, 807}, {773, 783}}, {{752, 760}, {728, 751}, {704, 727}, {694, 703}}, {{672, 680}, {648, 671}, {624, 647}, {314, 623}}, {{597, 605}, {578, 596}, {559, 577}, {552, 558}}, {{537, 545}, {518, 536}, {499, 517}, {491, 498}}, {{477, 485}, {458, 476}, {439, 457}, {430, 438}}, {{417, 423}, {398, 416}, {379, 397}, {370, 378}}, {{357, 360}, {338, 356}, {337, 319}, {313, 318}}, {{297, 305}, {278, 296}, {259, 277}, {250, 258}}, {{237, 253}, {218, 236}, {200, 217}, {184, 198}}, {{172, 188}, {156, 170}, {126, 147}, {0, 123}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int threes[23][4][2]     = { {{1910, 2001}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1882, 1905}, {1848, 1881}, {1814, 1845}, {1794, 1813}}, {{1762, 1780}, {1728, 1750}, {1695, 1717}, {1672, 1693}}, {{1642, 1659}, {1608, 1637}, {1575, 1606}, {1555, 1573}}, {{1522, 1539}, {1488, 1515}, {1455, 1486}, {1435, 1453}}, {{1407, 1419}, {1378, 1405}, {1352, 1376}, {1334, 1348}}, {{1307, 1319}, {1278, 1305}, {1253, 1277}, {1233, 1248}}, {{1207, 1220}, {1182, 1205}, {1150, 1173}, {1135, 1148}}, {{1107, 1020}, {1078, 1105}, {1050, 1077}, {1035, 1048}}, {{1007, 1020}, {978, 1005}, {951, 977}, {935, 948}}, {{912, 920}, {888, 911}, {864, 887}, {854, 863}}, {{832, 841}, {808, 831}, {784, 807}, {775, 783}}, {{752, 760}, {728, 751}, {704, 727}, {695, 703}}, {{672, 680}, {648, 671}, {624, 647}, {613, 623}}, {{597, 603}, {578, 596}, {559, 577}, {549, 558}}, {{537, 544}, {518, 536}, {499, 517}, {494, 498}}, {{477, 482}, {458, 476}, {439, 457}, {432, 438}}, {{417, 423}, {398, 416}, {379, 397}, {371, 378}}, {{357, 362}, {338, 356}, {319, 337}, {309, 318}}, {{297, 314}, {278, 296}, {259, 277}, {250, 258}}, {{237, 252}, {222, 236}, {202, 217}, {181, 198}}, {{175, 179}, {153, 169}, {142, 146}, {0, 118}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int hoops[23][4][2]      = { {{1355, 1362}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1337, 1346}, {1319, 1325}, {1303, 1314}, {1295, 1298}}, {{1278, 1292}, {1258, 1273}, {1240, 1252}, {1227, 1238}}, {{1217, 1228}, {1202, 1213}, {1180, 1196}, {1163, 1178}}, {{1157, 1165}, {1141, 1151}, {1120, 1136}, {1103, 1118}}, {{1097, 1109}, {1078, 1092}, {1060, 1074}, {1041, 1058}}, {{1037, 1048}, {1020, 1033}, {999, 1017}, {990, 998}}, {{977, 987}, {959, 974}, {939, 957}, {925, 938}}, {{917, 927}, {898, 916}, {879, 897}, {865, 878}}, {{857, 865}, {838, 856}, {819, 837}, {806, 818}}, {{797, 805}, {778, 796}, {759, 777}, {754, 758}}, {{737, 742}, {718, 736}, {699, 717}, {695, 698}}, {{677, 682}, {658, 676}, {639, 657}, {635, 638}}, {{617, 624}, {598, 616}, {579, 597}, {575, 578}}, {{557, 569}, {478, 496}, {459, 477}, {451, 458}}, {{497, 512}, {478, 496}, {459, 477}, {449, 458}}, {{437, 449}, {418, 436}, {400, 417}, {386, 398}}, {{377, 393}, {361, 375}, {343, 357}, {330, 338}}, {{317, 333}, {298, 316}, {280, 291}, {271, 271}}, {{-1, -1}, {238, 239}, {231, 231}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int rumble[23][4][2]     = { {{1343, 1423}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1337, 1354}, {1320, 1332}, {1300, 1317}, {1300, 1317}}, {{1277, 1286}, {1258, 1272}, {1240, 1255}, {1228, 1238}}, {{1217, 1226}, {1200, 1215}, {1180, 1197}, {1168, 1178}}, {{1157, 1165}, {1141, 1154}, {1120, 1137}, {1106, 1118}}, {{1097, 1105}, {1078, 1095}, {1059, 1077}, {1047, 1058}}, {{1037, 1047}, {1018, 1035}, {999, 1017}, {992, 998}}, {{977, 983}, {958, 976}, {939, 957}, {927, 938}}, {{917, 924}, {898, 916}, {879, 897}, {870, 878}}, {{857, 865}, {838, 856}, {819, 837}, {812, 818}}, {{797, 803}, {778, 796}, {759, 777}, {753, 758}}, {{737, 742}, {718, 736}, {699,717}, {692, 698}}, {{677, 683}, {658, 676}, {639, 657}, {635, 638}}, {{617, 620}, {598, 616}, {579, 597}, {575, 578}}, {{557, 563}, {478, 496}, {459, 477}, {449, 458}}, {{497, 513}, {478, 496}, {459, 417}, {447, 458}}, {{437, 453}, {418, 436}, {399, 417}, {381, 398}}, {{377, 394}, {358, 376}, {340, 357}, {323, 338}}, {{317, 332}, {304, 316}, {282, 297}, {267, 278}}, {{257, 273}, {238, 255}, {227, 236}, {207, 217}}, {{-1, -1}, {191, 192}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int dropshot[23][4][2]   = { {{1315, 1318}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1295, 1310}, {1286, 1291}, {1264, 1276}, {1241, 1255}}, {{1237, 1248}, {1220, 1227}, {1199, 1211}, {1181, 1198}}, {{1177, 1193}, {1158, 1175}, {1139, 1155}, {1126, 1138}}, {{1117, 1130}, {1098, 1115}, {1079, 1097}, {1065, 1078}}, {{1057, 1068}, {1038, 1054}, {1020, 1037}, {1005, 1018}}, {{997, 1011}, {978, 996}, {959, 977}, {955, 958}}, {{937, 943}, {918, 936}, {899, 917}, {890, 898}}, {{877, 887}, {858, 876}, {839, 857}, {827, 838}}, {{817, 825}, {798, 816}, {779, 797}, {775, 778}}, {{757, 772}, {738, 756}, {719, 737}, {715, 718}}, {{697, 704}, {678, 696}, {659, 677}, {655, 658}}, {{637, 643}, {618, 636}, {599, 617}, {595, 598}}, {{577, 585}, {558, 576}, {539, 557}, {524, 538}}, {{517, 534}, {498, 516}, {479, 497}, {471, 478}}, {{457, 473}, {438, 456}, {419, 437}, {402, 418}}, {{397, 408}, {378, 396}, {359, 377}, {341, 358}}, {{337, 352}, {319, 336}, {302, 317}, {283, 298}}, {{277, 280}, {259, 276}, {244, 254}, {226, 238}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int snowday[23][4][2]    = { {{1356, 1364}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1335, 1350}, {1321, 1332}, {1303, 1315}, {1291, 1301}}, {{1288, 1289}, {1258, 1274}, {1240, 1257}, {1233, 1236}}, {{1217, 1225}, {1200, 1215}, {1179, 1194}, {1168, 1178}}, {{1157, 1168}, {1138, 1149}, {1122, 1136}, {1108, 1118}}, {{1097, 1114}, {1078, 1093}, {1060, 1077}, {1045, 1058}}, {{1037, 1052}, {1018, 1035}, {999, 1017}, {998, 995}}, {{977, 987}, {958, 975}, {939, 957}, {927, 938}}, {{917, 926}, {898, 916}, {879, 897}, {875, 878}}, {{857, 868}, {838, 856}, {819, 837}, {815, 818}}, {{797, 811}, {778, 796}, {759, 777}, {755, 758}}, {{737, 743}, {718, 736}, {699, 717}, {684, 698}}, {{677, 691}, {658, 676}, {639, 657}, {635, 638}}, {{617, 622}, {598, 616}, {579, 597}, {563, 578}}, {{557, 566}, {538, 556}, {519, 537}, {513, 518}}, {{497, 508}, {478, 496}, {459, 477}, {455, 458}}, {{437, 452}, {418, 436}, {400, 417}, {383, 398}}, {{377, 388}, {362, 374}, {341, 357}, {327, 338}}, {{317, 325}, {300, 315}, {280, 292}, {266, 277}}, {{257, 263}, {241, 255}, {232, 235}, {217, 217}}, {{-1, -1}, {-1, -1}, {172, 172}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };
const int tournament[23][4][2] = { {{1901, 2166}, {-1, -1}, {-1, -1}, {-1, -1}}, {{1882, 1902}, {1848, 1878}, {1814, 1847}, {1781, 1813}}, {{1762, 1780}, {1728, 1760}, {1695, 1727}, {1661, 1693}}, {{1642, 1660}, {1608, 1641}, {1574, 1607}, {1507, 1574}}, {{1522, 1540}, {1488, 1521}, {1454, 1487}, {1420, 1453}}, {{1406, 1420}, {1378, 1406}, {1349, 1377}, {1320, 1348}}, {{1306, 1320}, {1278, 1306}, {1249, 1277}, {1220, 1248}}, {{1206, 1220}, {1178, 1206}, {1149, 1177}, {1120, 1148}}, {{1106, 1120}, {1078, 1106}, {1049, 1077}, {1020, 1048}}, {{1007, 1020}, {978, 1006}, {950, 977}, {921, 948}}, {{912, 920}, {888, 910}, {865, 887}, {841, 863}}, {{832, 840}, {808, 831}, {784, 807}, {761, 783}}, {{752, 760}, {728, 751}, {704, 727}, {681, 703}}, {{672, 680}, {648, 671}, {624, 647}, {613, 623}}, {{596, 600}, {576, 596}, {557, 577}, {541, 558}}, {{536, 540}, {517, 536}, {498, 517}, {481, 498}}, {{476, 480}, {457, 476}, {438, 457}, {421, 438}}, {{416, 420}, {397, 416}, {378, 397}, {361, 378}}, {{356, 360}, {338, 356}, {319, 337}, {301, 317}}, {{296, 300}, {278, 296}, {259, 277}, {243, 256}}, {{237, 240}, {218, 236}, {200, 216}, {181, 198}}, {{172, 180}, {149, 170}, {126, 147}, {0, 123}}, {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}} };

// Converts information into mmr. Format is directly from game (mode is 11-30, rank is 0-19, div is 0-3). Upper limit is to get the upper part of the range, setting it to false gets the lower part of the mmr range.
int RankViewer::unranker(int mode, int rank, int div, bool upperLimit) {

    int realRank, realDiv, realHeight;

    // Changes the rank to descending 
    int rankIndex[] = {22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    realRank = rankIndex[rank];

    // Since the divisions are descending in the array, we have to flip the index around
    int divIndex[] = { 3, 2, 1, 0 };
    realDiv = divIndex[div];

    auto divData = GetDivisionData((Playlist)mode, (Rank)realRank, realDiv);
    return upperLimit ? divData.higher : divData.lower;
}

// Gets the correct colors for each rank
void SetRankColor(int rank) {
    Rank realRank = (Rank)(rank);
    if (realRank < Rank::Unranked || realRank > Rank::SupersonicLegend)
    {
        colorScheme[0] = 0;
        colorScheme[1] = 0;
        colorScheme[2] = 0;
        
    }
    else {
        auto color = RankInfoDB[realRank].color;
        colorScheme[0] = color.r;
        colorScheme[1] = color.g;
        colorScheme[2] = color.b;
    }
}

std::string GetRankName(int rank, int div) {
    if (rank < 0 || rank > 22) {
        return "ERROR";
    }
    else {
        Rank realRank = (Rank)(rank);
        std::string rankName = RankInfoDB[realRank].name;
        // Screw you brank for not helping me
        if (rank != Rank::Unranked && rank != Rank::SupersonicLegend)
            rankName += " Div " + std::to_string(div + 1);

        return rankName;
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

    // Debug command
    cvarManager->registerNotifier("debug_mmr", [this](std::vector<std::string> args) {DebugGetDivisionData(args); }, "testing", PERMISSION_MENU);
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
                    nameCurrent = GetRankName(userTier, userDiv);
                    SetRankColor(userTier);
                    memcpy(colorCurrent, colorScheme, sizeof(colorScheme));

                    // Checks if the games are placement matches, so that the Next and Before don't show up
                    if (userTier <= 0) {
                        isPlacement = true;
                        drawCanvas = true;
                        return;
                    }
                    else {
                        isPlacement = false;
                    }

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
                    nameNext = GetRankName(upperTier, upperDiv);
                    SetRankColor(upperTier);
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
                    nameBefore = GetRankName(lowerTier, lowerDiv);
                    SetRankColor(lowerTier);
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

void RankViewer::loadMenu(std::string eventName)
{
    // Removes canvas if you quit the stats screen
    drawCanvas = false;
}

void RankViewer::onUnload()
{
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");
    gameWrapper->UnregisterDrawables();
}

DivisionData RankViewer::GetDivisionData(Playlist mode, Rank rank, int div)
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

#include "utils/parser.h"

void RankViewer::DebugGetDivisionData(std::vector<std::string> args)
{
    if (args.size() != 4)
    {
        cvarManager->log("Usage:" + args[0] + " mode rank div");
        return;
    }

    Playlist mode =  (Playlist) get_safe_int(args[1]);
    Rank rank = (Rank) get_safe_int(args[2]);
    int div = get_safe_int(args[3]);

    auto divData = GetDivisionData(mode, rank, div);
    cvarManager->log("Lower:" + std::to_string(divData.lower) + " Higher: " + std::to_string(divData.higher));
}
