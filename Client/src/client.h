#define OLC_PGEX_NETWORK
#include "olcPGEX/olcPGEX_Network.h"

#include "playerStruct.h"

class Client : public olc::net::client_interface<olc::net::ChatMsg>
{
public:
	Client() = delete;
	Client(const char* name, const char* ip, uint16_t port)
	{
		strncpy_s(clientInfo->name, name, sizeof(clientInfo->name));
		strcpy_s(clientInfo->message, "");
		if (!Connect(ip, port))
		{
			std::cout << "Error: Failed to connect to the server\n";
			return;
		}

		if (!IsConnected())
		{
			std::cout << "Error: Failed to connect to the server\n        Recheck the ip/port and try again!\n";
			return;
		}
	}
	
	void OnClientUpdate()
	{
		if (IsConnected())
		{
			// If our messageBuffer is not empty then
			// forward client's meesageBuffer to the server
			if (strcmp(clientInfo->message, ""))
			{
				olc::net::message<olc::net::ChatMsg> msg;
				msg.header.id = olc::net::ChatMsg::Server_UpdatePlayer;
				msg << *clientInfo;
				Send(msg);

				strcpy_s(clientInfo->message, "");
			}

			while (!Incoming().empty())
			{
				olc::net::message<olc::net::ChatMsg> msg = Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case(olc::net::ChatMsg::Client_Accepted):
				{
					std::cout << "Server accepted client - you're in!\n";
					olc::net::message<olc::net::ChatMsg> msg;
					msg.header.id = olc::net::ChatMsg::Client_RegisterWithServer;
					msg << *clientInfo;
					Send(msg);
					break;
				}

				case(olc::net::ChatMsg::Client_AssignID):
				{
					uint32_t playerID = 0;
					// Server is assigning us OUR id
					msg >> playerID;
					clientInfo->uniqueID = playerID;
					std::cout << "Assigned Client ID = [@" << playerID << "]\n";
					break;
				}

				case(olc::net::ChatMsg::Server_AddPlayer):
				{
					olc::net::playerStruct desc;
					msg >> desc;
					playerList.insert_or_assign(desc.uniqueID, desc);

					if (desc.uniqueID == clientInfo->uniqueID)
					{
						// Now we exist in game world
						m_WaitingForConnection = false;
					}
					break;
				}

				case(olc::net::ChatMsg::Server_RemovePlayer):
				{
					uint32_t nRemovalID = 0;
					msg >> nRemovalID;
					playerList.erase(nRemovalID);
					break;
				}

				case(olc::net::ChatMsg::Server_UpdatePlayer):
				{
					/*
					olc::net::playerStruct desc;
					msg >> desc;
					std::cout << desc.name;
					std::cout << "[@" << desc.uniqueID << "]: ";
					std::cout << desc.message << std::endl;
					break;*/
				}
				default:
					break;
				}
			}
		}
		else if (m_WaitingForConnection)
		{
			//std::cout << "Waiting to connect.............\n";
		}
	}
public:
	static std::unordered_map<uint32_t, olc::net::playerStruct> playerList;
private:
	s_PlayerInfo* clientInfo = s_PlayerInfo::Get();
	bool m_WaitingForConnection = true;
};