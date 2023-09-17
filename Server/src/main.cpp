#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>

#define OLC_PGEX_NETWORK
#include "olcPGEX/olcPGEX_Network.h"

uint32_t clientID = 10000;
const char* fileName = "secretChatHistory.txt";

class ChatServer : public olc::net::server_interface<olc::net::ChatMsg>
{
public:
	ChatServer(uint16_t port) : olc::net::server_interface<olc::net::ChatMsg>(port)
	{
	}

	std::unordered_map<uint32_t, olc::net::playerStruct> playerRoster;
	std::vector<uint32_t> IDLeft;

protected:
	bool OnClientConnect(std::shared_ptr<olc::net::connection<olc::net::ChatMsg>> client) override
	{
		// true means client is allowed to connect
		return true;
	}

	void OnClientValidated(std::shared_ptr<olc::net::connection<olc::net::ChatMsg>> client) override
	{
		// Client passed validation check, so send them a message informing
		// them they can continue to communicate
		olc::net::message<olc::net::ChatMsg> msg;
		msg.header.id = olc::net::ChatMsg::Client_Accepted;
		client->Send(msg);
	}

	void OnClientDisconnect(std::shared_ptr<olc::net::connection<olc::net::ChatMsg>> client) override
	{
		if (client)
		{
			if (playerRoster.find(client->GetID()) == playerRoster.end())
			{
				// client never added to roster, so just let it disappear
			}
			else
			{
				auto& pd = playerRoster[client->GetID()];
				std::cout << "[UNGRACEFUL REMOVAL]:" + std::to_string(pd.uniqueID) + "\n";
				playerRoster.erase(client->GetID());
				IDLeft.push_back(client->GetID());
			}
		}
	}

	void OnMessage(std::shared_ptr<olc::net::connection<olc::net::ChatMsg>> client, olc::net::message<olc::net::ChatMsg>& msg) override
	{
		if (!IDLeft.empty())
		{
			for (auto pid : IDLeft)
			{
				olc::net::message<olc::net::ChatMsg> m;
				m.header.id = olc::net::ChatMsg::Server_RemovePlayer;
				m << pid;
				std::cout << "Removing " << pid << "\n";
				MessageAllClients(m);
			}
			IDLeft.clear();
		}

		switch (msg.header.id)
		{
		case olc::net::ChatMsg::Client_RegisterWithServer:
		{ 
			olc::net::playerStruct desc;
			msg >> desc;
			desc.uniqueID = client->GetID();
			playerRoster.insert_or_assign(desc.uniqueID, desc);

			olc::net::message<olc::net::ChatMsg> msgSendID;
			msgSendID.header.id = olc::net::ChatMsg::Client_AssignID;
			msgSendID << ++clientID;
			MessageClient(client, msgSendID);

			olc::net::message<olc::net::ChatMsg> msgAddPlayer;
			msgAddPlayer.header.id = olc::net::ChatMsg::Server_AddPlayer;
			msgAddPlayer << desc;
			MessageAllClients(msgAddPlayer);

			for (const auto& player : playerRoster)
			{
				olc::net::message<olc::net::ChatMsg> msgAddOtherPlayers;
				msgAddOtherPlayers.header.id = olc::net::ChatMsg::Server_AddPlayer;
				msgAddOtherPlayers << player.second;
				MessageClient(client, msgAddOtherPlayers);
			}
			
			// Send it in buffers and client should receive in buffers
			olc::net::messageHistory messageHistoryBuffer;
			olc::net::message<olc::net::ChatMsg> msgHistory;
			msgHistory.header.id = olc::net::ChatMsg::Server_MessageHistorySent;
			std::ifstream myFile(fileName);
			while(!myFile.eof())
			{
				myFile.read(messageHistoryBuffer.messageBuffer, messageHistoryBuffer.bufferSize-1);
				messageHistoryBuffer.messageBuffer[myFile.gcount()] = '\0';
				messageHistoryBuffer.bufferSize = static_cast<uint32_t>(myFile.gcount());
				msgHistory << messageHistoryBuffer;
				MessageClient(client, msgHistory);
				
				// Clear the buffer and bufferSize to default
				std::memset(messageHistoryBuffer.messageBuffer, '\0', sizeof(messageHistoryBuffer.messageBuffer));
				messageHistoryBuffer.bufferSize = 1024;
			}
			myFile.close();
			break;
		}

		case olc::net::ChatMsg::Client_UnregisterWithServer:
		{
			break;
		}

		case olc::net::ChatMsg::Client_MessageSent:
		{
			// Simply bounce the message to everyone except the client which sent the message
			MessageAllClients(msg, client);

			// Log the message to server console
			olc::net::playerStruct desc;
			msg >> desc;
			if (desc.message[0] == '\0')
				break;
			std::cout << desc.name << "[@" << desc.uniqueID << "]: " << desc.message << std::endl;

			// Append to a file
			std::ofstream myFile(fileName, std::ios::app);
			if (myFile.is_open())
			{
				myFile << desc.name << "|"
					<< std::setprecision(2)
					<< desc.color[0] << '|'
					<< desc.color[1] << '|'
					<< desc.color[2] << '|'
					<< desc.color[3]
					<< "|" << desc.message << "\n";
				myFile.close();
			}
			break;
		}
		}
	}
};

int main()
{
	ChatServer server(60'000);
	server.Start();

	while (true)
	{
		// MaxMessages and Wait are the perimeters
		server.Update(-1, true);
	}

	system("pause");
}