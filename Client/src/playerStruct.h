#pragma once
#include <cstdint>

// unique to our player
class s_PlayerInfo
{
public:
    s_PlayerInfo();
    static s_PlayerInfo* Get();
public:
	uint32_t uniqueID = 0;
	char name[32] = "";
	char message[1024] = "";
	float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
private:
    static s_PlayerInfo* s_Instance;
};
