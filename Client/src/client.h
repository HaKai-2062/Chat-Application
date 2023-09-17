#pragma once

#include <fstream>
#include <iomanip>

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
			m_FailedTheConnection = true;
			std::cout << "Error: Failed to connect to the server\n";
			return;
		}

		if (!IsConnected())
		{
			m_FailedTheConnection = true;
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
				msg.header.id = olc::net::ChatMsg::Client_MessageSent;
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

				case(olc::net::ChatMsg::Client_MessageSent):
				{
					olc::net::playerStruct desc;
					msg >> desc;
					std::fstream uidlFile(fileName, std::fstream::app);
					if (uidlFile.is_open())
					{
						uidlFile << desc.name << ":"
							<< std::setprecision(2)
							<< desc.color[0] << ','
							<< desc.color[1] << ','
							<< desc.color[2] << ','
							<< desc.color[3]
							<< ":" << desc.message << "\n";
						uidlFile.close();
					}
					break;
				}
				default:
					break;
				}
			}
		}
	}

	bool HasClientConnected() { return !m_WaitingForConnection; }
	bool HasConnectionFailed() { return m_FailedTheConnection; }
public:
	// Should set a getter for this
	static std::unordered_map<uint32_t, olc::net::playerStruct> playerList;
private:
	s_PlayerInfo* clientInfo = s_PlayerInfo::Get();
	bool m_WaitingForConnection = true;
	bool m_FailedTheConnection = false;
};