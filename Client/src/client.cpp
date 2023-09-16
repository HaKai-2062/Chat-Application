#include <iostream>

#include "olcPGEX/olcPGEX_Network.h"

struct s_PlayerInfo
{
	uint32_t uniqueID;
	char playerName[64];
	char playerMessage[1024];
};

class Client : public olc::net::client_interface<ChatMsg>
{
public:
	Client() = delete;
	Client(const std::string& ip, uint16_t port)
	{
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
	
		std::cout << "IP found\n";
	}
	void OnUserUpdate()
	{
		loopNumber++;
	
		if (loopNumber < 6)
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		else
		{
			// Begin chatting
			std::cout << m_PlayerInfo.playerName << "[@" << m_PlayerInfo.uniqueID << "]: ";
			std::cin.getline(m_PlayerInfo.playerMessage, sizeof(m_PlayerInfo.playerMessage));
	
			olc::net::message<ChatMsg> msg;
			msg.header.id = ChatMsg::Server_UpdatePlayer;
			msg << m_PlayerInfo;
	
			// use imgui to display stuff to everyone
			Send(msg);
		}
	
		if (IsConnected())
		{
			while (!Incoming().empty())
			{
				olc::net::message<ChatMsg> msg = Incoming().pop_front().msg;
	
				switch (msg.header.id)
				{
				case(ChatMsg::Client_Accepted):
				{
					std::cout << "Server accepted client - you're in!\n";
					olc::net::message<ChatMsg> msg;
					msg.header.id = ChatMsg::Client_RegisterWithServer;
					msg << m_PlayerInfo;
					Send(msg);
					break;
				}
	
				case(ChatMsg::Client_AssignID):
				{
					uint32_t playerID = 0;
					// Server is assigning us OUR id
					msg >> playerID;
					m_PlayerInfo.uniqueID = playerID;
					std::cout << "Assigned Client ID = [@" << playerID << "]\n";
					break;
				}
	
				case(ChatMsg::Server_AddPlayer):
				{
					s_PlayerInfo desc;
					msg >> desc;
					mapObjects.insert_or_assign(desc.uniqueID, desc);
	
					if (desc.uniqueID == m_PlayerInfo.uniqueID)
					{
						// Now we exist in game world
						m_WaitingForConnection = false;
					}
					break;
				}
	
				case(ChatMsg::Server_RemovePlayer):
				{
					uint32_t nRemovalID = 0;
					msg >> nRemovalID;
					mapObjects.erase(nRemovalID);
					break;
				}
	
				case(ChatMsg::Server_UpdatePlayer):
				{
					s_PlayerInfo desc;
					msg >> desc;
					std::cout << desc.playerName;
					std::cout << "[@" << desc.uniqueID << "]: ";
					std::cout << desc.playerMessage << std::endl;
					// mapObjects.insert_or_assign(desc.uniqueID, desc);
					break;
				}
				default:
					break;
				}
			}
		}
		else if (m_WaitingForConnection)
		{
			std::cout << "Waiting to connect.............\n";
		}
	}
private:
	std::unordered_map<uint32_t, s_PlayerInfo> mapObjects;
	static s_PlayerInfo m_PlayerInfo;
	bool m_WaitingForConnection = true;
	uint8_t loopNumber = 0;
};