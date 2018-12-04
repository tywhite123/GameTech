#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include <string>
#include <fstream>
#include "../CSC8503Common/PlayerObject.h"

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

		};
	}
}

