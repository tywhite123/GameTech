#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include <string>
#include <fstream>
#include "../CSC8503Common/PlayerObject.h"
#include "../CSC8503Common/GameClient.h"
#include "PacketReceiver.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/PushdownMachine.h"

namespace NCL {
	namespace CSC8503 {

		typedef std::pair<int, GameObject*> DynamicObject;

		class GolfGame
		{
		public:
			GolfGame();
			~GolfGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();
			void InitialiseNetwork();
			void SetupPushdown();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void LoadLevel(std::string file);

			bool SelectObject();
			void MoveSelectedObject();

			void SetPlayer(GameObject* player);

			GameObject* AddPlayerToWorld(int objID, const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddWallToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddGoalToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddMovingToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddRobotToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddSpinnerToWorld(int objID, const Vector3&position, Vector3 dimensions, float inverseMass = 10.0f, float spinVal = 10.0f);
			GameObject* AddFloorToWorld(int objID, const Vector3& position, Vector3 dimensions);

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	cubeMesh = nullptr;
			OGLMesh*	sphereMesh = nullptr;
			OGLTexture* basicTex = nullptr;
			OGLShader*	basicShader = nullptr;

			Level* level;

			GameClient* client;
			bool connected;
			bool printed = false;
			StringPacketReceiver clientReceiver;
			ObjectPacketReceiver objectDataReceiver;
			AllPlayersReadyReceiver allPlayersReadyReceiver;

			int playerID;
			string playerName;
			int playerPushes;

			vector<StateMachine*> stateMachines;

			int cameraDist;
			bool freeCam;


			StateType state;
			int menuSelection;
			bool selected[4];
			StateMachine* sMachine;

			bool allReady;
			vector<UpdateData*> updateData;

			std::map<int, GameObject*> dynamicObjects;
			

		};
	}
}

