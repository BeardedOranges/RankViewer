#pragma once
#include <map>
#include "RankEnums.h"

struct DivisionData
{
	int lower;
	int higher;
};

struct TierData
{
	std::vector<DivisionData> divisions;
};

struct  PlaylistData
{
	std::vector<TierData> tiers;
};

struct Color { int r, g, b; };

struct RankInfo { Color color; std::string name; };

extern std::map<Playlist, PlaylistData&> playlistMMRDatabase;
extern std::map<Rank, RankInfo> RankInfoDB;