#include "GolfGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../CSC8503Common/PositionConstraint.h"
#include <regex>
#include "../CSC8503Common/PlayerObject.h"
#include "PacketReceiver.h"
#include "MovingWallObject.h"
#include "../../Common/Maths.h"
#include "RobotObject.h"
#include "SpinnerObject.h"
#include "../CSC8503Common/PushdownState.h"

Level* Level::instance = 0;

GolfGame::GolfGame()
{
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = false;
	inSelectionMode = false;
	playerPushes = 0;
	cameraDist = 40;
	freeCam = false;
	state = Main_Menu;
	menuSelection = 0;
	for (int i = 0; i < 4; ++i)
		selected[i] = false;

	sMachine = new StateMachine();

	allReady = false;


	Debug::SetRenderer(renderer);
	level = Level::GetInstance();
	InitialiseAssets();
	InitialiseNetwork();
	SetupPushdown();
 
}


GolfGame::~GolfGame()
{
	delete cubeMesh;
	delete sphereMesh;
	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;

	NetworkBase::Destroy();
}

void GolfGame::InitialiseAssets()
{
	cubeMesh = new OGLMesh("cube.msh");
	cubeMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	cubeMesh->UploadToGPU();

	sphereMesh = new OGLMesh("sphere.msh");
	sphereMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	sphereMesh->UploadToGPU();

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	InitWorld();
}

void GolfGame::InitialiseNetwork()
{
	NetworkBase::Initialise();

	string name;
	std::cout << "Please Enter your username: ";
	std::cin >> name;

	playerName = name;

	int port = NetworkBase::GetDefaultPort();

	client = new GameClient();
	client->SetPeerID(playerID);
	clientReceiver = StringPacketReceiver(playerName);
	objectDataReceiver = ObjectPacketReceiver(playerName, updateData);
	allPlayersReadyReceiver = AllPlayersReadyReceiver(playerName, allReady);

	client->RegisterPacketHandler(String, &clientReceiver);
	client->RegisterPacketHandler(Object_Data, &objectDataReceiver);
	client->RegisterPacketHandler(All_Players_Ready, &allPlayersReadyReceiver);
	client->SetName(name);

	//TODO: move to where the player select multiplayer
	connected = client->Connect(127, 0, 0, 1, port);



	

}

void GolfGame::SetupPushdown()
{	
	StateFunc mainMenu = [](void* data)
	{
		GolfGame* g = (GolfGame*)data;
		if(Window::GetKeyboard()->KeyPressed(KEYBOARD_DOWN))
		{
			g->menuSelection++;
			if (g->menuSelection >= 4)
				g->menuSelection = 0;
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_UP))
		{
			g->menuSelection--;
			if (g->menuSelection < 0)
				g->menuSelection = 3;
		}

		if(g->menuSelection == 0)
		{
			g->selected[0] = true;
			g->selected[1] = false;
			g->selected[2] = false;
			g->selected[3] = false;
		}
		else if(g->menuSelection == 1)
		{
			g->selected[0] = false;
			g->selected[1] = true;
			g->selected[2] = false;
			g->selected[3] = false;
		}
		else if (g->menuSelection == 2)
		{
			g->selected[0] = false;
			g->selected[1] = false;
			g->selected[2] = true;
			g->selected[3] = false;
		}
		else if (g->menuSelection == 3)
		{
			g->selected[0] = false;
			g->selected[1] = false;
			g->selected[2] = false;
			g->selected[3] = true;
		}

		if(Window::GetKeyboard()->KeyPressed(KEYBOARD_RETURN))
		{
			//return PushdownState::Push;
			if (g->menuSelection == 0)
			{
				g->state = Game;
			}
			else if (g->menuSelection == 1)
			{
				g->state = Lobby;
			}
			else if (g->menuSelection == 2)
			{
				//options
			}
			else if (g->menuSelection == 3)
			{
				//exit
			}

		}

	};

	StateFunc game = [](void* data)
	{
		GolfGame* g = (GolfGame*)data;
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_P))
			g->state = Pause;
	};

	StateFunc pause = [](void* data)
	{
		GolfGame* g = (GolfGame*)data;
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_P))
			g->state = Game;
	};

	StateFunc lobby = [](void* data)
	{
		GolfGame* g = (GolfGame*)data;
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_RETURN))
		{
			//TODO: ready up serveer stuff
			g->state = Game;
		}

		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_BACK))
		{
			g->state = Main_Menu;
		}
	};

	State* menuState = new GenericState(mainMenu, (void*)this);
	sMachine->AddState(menuState);

	State* gameState = new GenericState(game, (void*)this);
	sMachine->AddState(gameState);

	State* pauseState = new GenericState(pause, (void*)this);
	sMachine->AddState(pauseState);

	State* lobbyState = new GenericState(lobby, (void*)this);
	sMachine->AddState(lobbyState);


	GenericTransition<int&, int>* menuA = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::EqualsTransition, (int&)state, Game, menuState, gameState);

	GenericTransition<int&, int>* menuB = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::EqualsTransition, (int&)state, Lobby, menuState, lobbyState);

	GenericTransition<int&, int>* gameA = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::EqualsTransition, (int&)state, Pause, gameState, pauseState);

	GenericTransition<int&, int>* pauseA = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::EqualsTransition, (int&)state, Game, pauseState, gameState);

	GenericTransition<int&, int>* lobbyA = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::EqualsTransition, (int&)state, Game, lobbyState, gameState);

	GenericTransition<int&, int>* lobbyB = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::EqualsTransition, (int&)state, Main_Menu, lobbyState, menuState);

	sMachine->AddTransition(menuA);
	sMachine->AddTransition(gameA);
	sMachine->AddTransition(pauseA);
	sMachine->AddTransition(menuB);
	sMachine->AddTransition(lobbyA);
	sMachine->AddTransition(lobbyB);


}

void GolfGame::UpdateGame(float dt)
{
	
	if(state == Main_Menu)
	{
		renderer->DrawString("Golf wit' Lads!", Vector2(100, 600), Vector4(0, 1, 0, 1));
		renderer->DrawString(playerName, Vector2(900 - playerName.length(), 600), Vector4(0, 0, 0, 1));
		renderer->DrawString("Single Player", Vector2(100, 350), Vector4(0, selected[0], 0, 1));
		renderer->DrawString("Multi Player", Vector2(100, 300), Vector4(0, selected[1], 0, 1));
		renderer->DrawString("Options", Vector2(100, 250), Vector4(0, selected[2], 0, 1));
		renderer->DrawString("Exit", Vector2(100, 200), Vector4(0, selected[3], 0, 1));
		world->UpdateWorld(dt);
		renderer->Update(dt);
		renderer->Render();
	}
	else if(state == Game){
		renderer->DrawString("Render Time: " + std::to_string(1000.0f*dt), Vector2(10, 720 - 60));
		renderer->DrawString("Score: " + std::to_string(playerPushes) + "!", Vector2(10, 720 - 100), Vector4(1, 1, 1, 1));

		//Move to an update Camera
		if (!freeCam) {
			float camX = cameraDist * sinf(world->GetMainCamera()->GetYaw() * PI / 180) * cosf(world->GetMainCamera()->GetPitch() * PI / 180);
			float camY = cameraDist * -sinf(world->GetMainCamera()->GetPitch() * PI / 180);
			float camZ = cameraDist * cosf(world->GetMainCamera()->GetYaw() * PI / 180) * cosf(world->GetMainCamera()->GetPitch() * PI / 180);

			world->GetMainCamera()->SetPosition(selectionObject->GetTransform().GetWorldPosition() + Vector3(camX, camY, camZ));
		}

		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
		}

		UpdateKeys();

		if (useGravity) {
			Debug::Print("(G)ravity on", Vector2(10, 40));
		}
		else {
			Debug::Print("(G)ravity off", Vector2(10, 40));
		}

		//SelectObject();
		MoveSelectedObject();

		world->UpdateWorld(dt);
		renderer->Update(dt);
		physics->Update(dt);

		for (StateMachine*s : stateMachines)
			s->Update();


		if (level->loadNext) {
			renderer->DrawString("Level Finished", Vector2(1280 / 3, 720 / 2), Vector4(1, 1, 1, 1));
			renderer->DrawString("Press Enter to Load Next Level!", Vector2(1280 / 6, (720 / 2) - 40), Vector4(1, 1, 1, 1));
			renderer->DrawString("Score: " + std::to_string(playerPushes) + "!", Vector2(1280 / 3, (720 / 2) - 80), Vector4(1, 1, 0, 1));

			printed = false;

		}



		if (connected)
		{

			if (level->loadNext && !printed) {
				client->SendPacket(StringPacket(playerName + " finished level " + std::to_string(level->GetLevel() - 1)));
				client->SendPacket(ScorePacket(playerPushes));
				printed = true;

			}
			client->UpdateClient();


			//More efficient way?
			for(UpdateData* d: updateData)
			{
				GameObject* obj = dynamicObjects.at(d->objID);
				obj->GetTransform().SetWorldPosition(d->pos);
				obj->GetTransform().SetLocalOrientation(d->ori);
			}
			updateData.clear();
		}

		Debug::FlushRenderables();
		renderer->Render();
	}
	else if(state == Pause)
	{
		renderer->DrawString("Pause!", Vector2(100, 600), Vector4(0, 1, 0, 1));
		renderer->DrawString("Press P to Exit Pause!", Vector2(100, 200), Vector4(0, 1, 0, 1));
		world->UpdateWorld(dt);
		renderer->Update(dt);
		renderer->Render();
	}
	else if(state == Lobby)
	{
		renderer->DrawString("Multiplayer Lobby!", Vector2(100, 600), Vector4(0, 1, 0, 1));
		renderer->DrawString("Players Connected: ", Vector2(100, 500), Vector4(0, 1, 0, 1));
		renderer->DrawString(playerName, Vector2(100, 450), Vector4(0, 0, 0, 1));
		//client->UpdateClient();
		

		world->UpdateWorld(dt);
		renderer->Update(dt);
		renderer->Render();

	}

	sMachine->Update();
}

void GolfGame::UpdateKeys()
{

	if(Window::GetMouse()->ButtonHeld(MOUSE_MIDDLE))
	{
		inSelectionMode = false;
		Window::GetWindow()->ShowOSPointer(false);
		Window::GetWindow()->LockMouseToWindow(true);
		
	}
	else {
		inSelectionMode = true;
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
		
	}


	if (Window::GetKeyboard()->KeyPressed(KEYBOARD_F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KEYBOARD_F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::KEYBOARD_G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}


	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KEYBOARD_F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KEYBOARD_F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KEYBOARD_F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KEYBOARD_F8)) {
		world->ShuffleObjects(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KEYBOARD_F)) {
		freeCam = !freeCam;
	}
	
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP)) {
		cameraDist -= 1;
		//selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -100));
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN)) {
		cameraDist += 1;
		//selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 100));
	}

	if (level->loadNext)
	{
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RETURN))
		{
			stateMachines.clear();
			InitWorld();
			level->loadNext = false;
		}
	}

}

void GolfGame::InitCamera()
{
	world->GetMainCamera()->SetNearPlane(3.0f);
	world->GetMainCamera()->SetFarPlane(4200.0f);
	world->GetMainCamera()->SetPitch(-35.0f);
	world->GetMainCamera()->SetYaw(320.0f);
	world->GetMainCamera()->SetPosition(Vector3(-100, 120, 200));
}

void GolfGame::InitWorld()
{
	world->ClearAndErase();
	physics->Clear();

	LoadLevel("TestLevel" + std::to_string(level->GetLevel()) + ".txt");
	printed = false;
	playerPushes = 0;

}


/**
*	Level File Format Key:
*	S - Ball Start
*	E - End/Goal
*	x - Wall Cube
*	. - Empty Space
*	m - Moving Wall
*/
void GolfGame::LoadLevel(std::string filename)
{
	std::ifstream file("../../Assets/Data/" + filename);
	int width = 0;
	int depth = 0;
	int x, y, z;
	Vector3 cubeDims;
	int i = 0;
	int objID = 0;
	if (file.is_open())
	{
		string line;
		while (getline(file, line)) {
			if (std::regex_match(line, std::regex("[0-9]+")))
			{
				switch (i)
				{
				case 0: x = stoi(line); break;
				case 1: y = stoi(line); break;
				case 2:
					z = stoi(line);
					cubeDims = Vector3(x, y, z);
					break;
				}
				i++;
			}
			else
			{
				for (char& in : line) {
					Vector3 pos(width*(cubeDims.x * 2), 0, depth*(cubeDims.z * 2));
					if (in == 'x')
					{
						AddWallToWorld(objID, pos, cubeDims, 0);
					}
					else if (in == 'S')
					{
						AddPlayerToWorld(objID, pos, cubeDims.x * 0.5f, 10.0f);
						
					}
					else if (in == 'E')
					{
						AddGoalToWorld(objID, pos, cubeDims*0.5f, 0);
					}
					else if(in == 'm')
					{
						AddMovingToWorld(objID, pos, Vector3(cubeDims.x*0.25f, cubeDims.y*0.35f, cubeDims.z), 1.f);
						
					}
					else if(in == 'r')
					{
						AddRobotToWorld(objID, pos, cubeDims*0.5f, 0);
					}
					else if(in == 's')
					{
						AddSpinnerToWorld(objID, pos, Vector3(cubeDims.x*0.15f, cubeDims.y*0.35f, cubeDims.z), 10.0f, 10.0f);
					}
					width++;
					objID++;
				}
				depth++;
				width = 0;
			}

		}
	}
	file.close();
	AddFloorToWorld(objID, Vector3((cubeDims.x*cubeDims.z) - cubeDims.x, -(cubeDims.y * 2)/*-cubeDims.y*/, (cubeDims.x*cubeDims.z) - cubeDims.z), Vector3((cubeDims.x*cubeDims.z), 10, (cubeDims.x*cubeDims.z)));
}

//bool GolfGame::SelectObject()
//{
//	if (Window::GetKeyboard()->KeyPressed(KEYBOARD_Q)) {
//		inSelectionMode = !inSelectionMode;
//		if (inSelectionMode) {
//			Window::GetWindow()->ShowOSPointer(true);
//			Window::GetWindow()->LockMouseToWindow(false);
//		}
//		else {
//			Window::GetWindow()->ShowOSPointer(false);
//			Window::GetWindow()->LockMouseToWindow(true);
//		}
//	}
//	if (inSelectionMode) {
//		renderer->DrawString("Press Q to change to camera mode!", Vector2(10, 0));
//
//		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::MOUSE_LEFT)) {
//			if (selectionObject) {	//set colour to deselected;
//				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
//				selectionObject = nullptr;
//			}
//
//			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
//			Debug::DrawLine(ray.GetPosition(), ray.GetPosition() + (ray.GetDirection() * 1000), Vector4(1, 0, 0, 1));
//
//			RayCollision closestCollision;
//			if (world->Raycast(ray, closestCollision, true)) {
//				selectionObject = (GameObject*)closestCollision.node;
//				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
//				return true;
//			}
//			else {
//				return false;
//			}
//		}
//	}
//	else {
//		renderer->DrawString("Press Q to change to select mode!", Vector2(10, 0));
//	}
//	return false;
//}

void GolfGame::MoveSelectedObject()
{
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude), Vector2(10, 20));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::MOUSE_RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		Debug::DrawLine(ray.GetPosition(), ray.GetPosition() + (ray.GetDirection() * 1000), Vector4(1, 0, 0, 1));

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {

				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
				client->SendPacket(BallForcePacket(selectionObject->GetObjID(), ray.GetDirection() * forceMagnitude, closestCollision.collidedAt));
				if(selectionObject->GetName() == "Player" && !level->loadNext)
					playerPushes++;
			}
		}
	}
}



/**
 *	Functions for adding objects to the world
 */


void GolfGame::SetPlayer(GameObject * player)
{
	selectionObject = player;
	selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
}

GameObject* GolfGame::AddPlayerToWorld(int objID, const Vector3 & position, float radius, float inverseMass)
{
	PlayerObject* player = new PlayerObject(objID, "Player");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	player->SetBoundingVolume((CollisionVolume*)volume);
	player->GetTransform().SetWorldScale(sphereSize);
	player->GetTransform().SetWorldPosition(position);
	player->SetStartingPos(position);

	player->SetRenderObject(new RenderObject(&player->GetTransform(), sphereMesh, basicTex, basicShader));
	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));

	player->GetPhysicsObject()->SetInverseMass(inverseMass);
	player->GetPhysicsObject()->InitSphereInertia();
	player->GetPhysicsObject()->SetElasticity(0.66f);
	player->GetPhysicsObject()->SetCanImpulse(true);
	player->GetPhysicsObject()->SetAffectedByGrav(true);
	player->level = level;

	dynamicObjects.insert(DynamicObject(objID, (GameObject*)player));

	world->AddGameObject((GameObject*)player);
	SetPlayer(player);

	return player;
}

GameObject* GolfGame::AddWallToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	GameObject* wall = new GameObject(objID, "Wall");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	wall->SetBoundingVolume((CollisionVolume*)volume);

	wall->GetTransform().SetWorldPosition(position);
	wall->GetTransform().SetWorldScale(dimensions);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(inverseMass);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetPhysicsObject()->SetElasticity(1.66f);
	wall->GetPhysicsObject()->SetCanImpulse(false);

	wall->GetRenderObject()->SetColour(Vector4(0.3f, 0.1f, 0.1f, 1));

	world->AddGameObject(wall);

	return wall;
}

GameObject* GolfGame::AddGoalToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	GameObject* goal = new GameObject(objID, "Goal");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	goal->SetBoundingVolume((CollisionVolume*)volume);

	goal->GetTransform().SetWorldPosition(position);
	goal->GetTransform().SetWorldScale(dimensions);

	goal->SetRenderObject(new RenderObject(&goal->GetTransform(), cubeMesh, basicTex, basicShader));
	goal->SetPhysicsObject(new PhysicsObject(&goal->GetTransform(), goal->GetBoundingVolume()));

	goal->GetPhysicsObject()->SetInverseMass(inverseMass);
	goal->GetPhysicsObject()->InitCubeInertia();

	goal->GetPhysicsObject()->SetElasticity(1.66f);
	goal->GetPhysicsObject()->SetPhysical(false);

	world->AddGameObject(goal);

	return goal;
}

GameObject * GolfGame::AddMovingToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	MovingWallObject* movingWall = new MovingWallObject(objID, "Moving Wall");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	movingWall->SetBoundingVolume((CollisionVolume*)volume);

	movingWall->GetTransform().SetWorldPosition(position);
	movingWall->GetTransform().SetWorldScale(dimensions);

	movingWall->SetRenderObject(new RenderObject(&movingWall->GetTransform(), cubeMesh, basicTex, basicShader));
	movingWall->SetPhysicsObject(new PhysicsObject(&movingWall->GetTransform(), movingWall->GetBoundingVolume()));

	movingWall->GetPhysicsObject()->SetInverseMass(inverseMass);
	movingWall->GetPhysicsObject()->InitCubeInertia();

	movingWall->GetPhysicsObject()->SetElasticity(1.66f);
	movingWall->GetPhysicsObject()->SetPhysical(true);
	movingWall->GetPhysicsObject()->SetCanImpulse(false);
	movingWall->GetPhysicsObject()->SetAffectedByGrav(false);

	movingWall->SetupStateMachine();

	stateMachines.push_back(movingWall->GetStateMachine());

	dynamicObjects.insert(DynamicObject(objID, (GameObject*)movingWall));

	world->AddGameObject((GameObject*)movingWall);

	return movingWall;
}

GameObject * GolfGame::AddRobotToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	RobotObject* robot = new RobotObject(objID, "Robot");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	robot->SetBoundingVolume((CollisionVolume*)volume);

	robot->GetTransform().SetWorldPosition(position);
	robot->GetTransform().SetWorldScale(dimensions);

	robot->SetRenderObject(new RenderObject(&robot->GetTransform(), cubeMesh, basicTex, basicShader));
	robot->SetPhysicsObject(new PhysicsObject(&robot->GetTransform(), robot->GetBoundingVolume()));

	robot->GetPhysicsObject()->SetInverseMass(inverseMass);
	robot->GetPhysicsObject()->InitCubeInertia();

	robot->GetPhysicsObject()->SetElasticity(0.f);
	robot->GetPhysicsObject()->SetPhysical(true);
	robot->GetPhysicsObject()->SetCanImpulse(false);
	robot->GetPhysicsObject()->SetAffectedByGrav(false);

	robot->SetupStateMachine();

	stateMachines.push_back(robot->GetStateMachine());

	dynamicObjects.insert(DynamicObject(objID, (GameObject*)robot));

	world->AddGameObject((GameObject*)robot);

	return robot;
}

GameObject* GolfGame::AddSpinnerToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass, float spinVal)
{
	SpinnerObject* spinner = new SpinnerObject(objID, "Spinner", spinVal);
	OBBVolume* volume = new OBBVolume(dimensions);

	spinner->SetBoundingVolume((CollisionVolume*)volume);

	spinner->GetTransform().SetWorldPosition(position);
	spinner->GetTransform().SetWorldScale(dimensions);

	spinner->SetRenderObject(new RenderObject(&spinner->GetTransform(), cubeMesh, basicTex, basicShader));
	spinner->SetPhysicsObject(new PhysicsObject(&spinner->GetTransform(), spinner->GetBoundingVolume()));

	spinner->GetPhysicsObject()->SetInverseMass(inverseMass);
	spinner->GetPhysicsObject()->InitCubeInertia();

	spinner->GetPhysicsObject()->SetElasticity(0.3f);
	spinner->GetPhysicsObject()->SetPhysical(true);
	spinner->GetPhysicsObject()->SetCanImpulse(false);
	spinner->GetPhysicsObject()->SetAffectedByGrav(false);

	spinner->SetupStateMachine();

	stateMachines.push_back(spinner->GetStateMachine());

	dynamicObjects.insert(DynamicObject(objID, (GameObject*)spinner));

	world->AddGameObject((GameObject*)spinner);

	return spinner;
}

GameObject* GolfGame::AddFloorToWorld(int objID, const Vector3 & position, Vector3 dimensions)
{
	GameObject* floor = new GameObject(objID, "Floor");
	AABBVolume* volume = new AABBVolume(dimensions);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(dimensions);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	world->AddGameObject(floor);

	return floor;
}
