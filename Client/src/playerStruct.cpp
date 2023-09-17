#include "playerStruct.h"
#include <string.h>

s_PlayerInfo* s_PlayerInfo::s_Instance = nullptr;


s_PlayerInfo::s_PlayerInfo()
{
	if (s_Instance)
		return;
	
	s_Instance = this;
}

s_PlayerInfo* s_PlayerInfo::Get()

{
    if (s_Instance == nullptr) 
        s_Instance = new s_PlayerInfo();
    return s_Instance;
}