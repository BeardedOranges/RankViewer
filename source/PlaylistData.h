#pragma once
#include <map>
#include "RankEnums.h"
/*
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
};*/

struct FName2
{
	int32_t Index;
	int32_t Instance;
};

struct RankInfo { std::string name; };

//extern std::map<Playlists, PlaylistData&> playlistMMRDatabase;
extern std::map<Rank, RankInfo> RankInfoDB;