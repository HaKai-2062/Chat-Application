#include <iostream>
#define OLC_PGEX_NETWORK
#include "olcPGEX/olcPGEX_Network.h"

uint32_t clientID = 0;

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

			break;
		}

		case olc::net::ChatMsg::Client_UnregisterWithServer:
		{
			break;
		}

		case olc::net::ChatMsg::Server_UpdatePlayer:
		{
			olc::net::playerStruct desc;
			msg >> desc;
			if (desc.playerMessage[0] == '\0')
				break;
			std::cout << desc.playerName << "[@" << desc.uniqueID << "]: " << desc.playerMessage << std::endl;
			// Simply bounce update to everyone except incoming client
			MessageAllClients(msg, client);
			break;
		}

		}
	}
};

int main()
{
	ChatServer server(60000);
	server.Start();

	while (true)
	{
		// MaxMessages and Wait are the perimeters
		server.Update(-1, true);
	}

	system("pause");
}