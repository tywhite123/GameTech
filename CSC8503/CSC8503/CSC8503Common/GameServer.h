#pragma once
#include <thread>
#include <atomic>

#include "NetworkBase.h"
#include <vector>

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients);
			~GameServer();

			bool Initialise();
			void Shutdown();

			void SetPlayerVars(std::vector<int>& iDs, std::map<int, std::string>& p)
			{
				playerIDs = &iDs;
				players = &p;
			}

			void SetNewPlayer(bool& p) { newPlayer = &p; }

			void SetGameWorld(GameWorld &g);

			void ThreadedUpdate();

			bool SendGlobalMessage(int msgID);
			bool SendGlobalMessage(GamePacket& packet);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();
			
			virtual void UpdateServer();

			int GetClientCount() const { return clientCount; }

		protected:
			int			port;
			int			clientMax;
			int			clientCount;
			GameWorld*	gameWorld;

			std::atomic<bool> threadAlive;

			std::vector<int>* playerIDs;
			std::map<int, std::string>* players;
			bool* newPlayer;

			

			std::thread updateThread;

			int incomingDataRate;
			int outgoingDataRate;

			std::map<int, int> stateIDs;
		};
	}
}
