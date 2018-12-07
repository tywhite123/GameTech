#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include <string>
#include <fstream>
#include "../CSC8503Common/PlayerObject.h"
#include "../CSC8503Common/GameClient.h"
#include "PacketReceivers.h"
#include "../CSC8503Common/StateMachine.h"

namespace NCL {
	namespace CSC8503 {
		class GolfGame
		{
		public:
			GolfGame();
			~GolfGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();
			void InitialiseNetwork();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void LoadLevel(std::string file);

			bool SelectObject();
			void MoveSelectedObject();

			void SetPlayer(GameObject* player);

			GameObject* AddPlayerToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddGoalToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddMovingToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddRobotToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddFloorToWorld(const Vector3& position);

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
			PacketReceivers clientReceiver;

			string playerName;
			int playerPushes;

			vector<StateMachine*> stateMachines;

			int cameraDist;
			bool freeCam;
		};
	}
}

