#pragma once
#include <cstdint>

// unique to our player
class s_PlayerInfo
{
public:
    s_PlayerInfo();
    static s_PlayerInfo* Get();
public:
	uint32_t uniqueID;
	char playerName[32];
	char playerMessage[1024];
private:
    static s_PlayerInfo* s_Instance;
};
