#include "../../Common/Window.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#include "../CSC8503Common/NavigationGrid.h"

#include "TutorialGame.h"
//#include "NetworkedGame.h"
//#include "GolfGame.h"

using namespace NCL;
using namespace CSC8503;

void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();

	int someData = 0;

	StateFunc AFunc = [](void*data)
	{
		int* realData = (int*)data;
		(*realData)++;
		std::cout << "In State A!" << std::endl;
	};
	StateFunc BFunc = [](void*data)
	{
		int* realData = (int*)data;
		(*realData)--;
		std::cout << "	In State B!" << std::endl;
	};

	GenericState* stateA = new GenericState(AFunc, (void*)&someData);
	GenericState* stateB = new GenericState(BFunc, (void*)&someData);
	testMachine->AddState(stateA);
	testMachine->AddState(stateB);

	GenericTransition<int&, int>* transitionA = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::GreaterThanTransition, someData, 10, stateA, stateB);

	GenericTransition<int&, int>* transitionB = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::EqualsTransition, someData, 0, stateB, stateA);

	testMachine->AddTransition(transitionA);
	testMachine->AddTransition(transitionB);

	for (int i = 0; i < 100; ++i)
		testMachine->Update();
	delete testMachine;


}


class TestPacketReceiver : public PacketReceiver
{
public:
	TestPacketReceiver(string name)
	{
		this->name = name;
	}

	string GetName() { return name; }

	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if(type == String)
		{
			StringPacket* realPacket = (StringPacket*)payload;
			string msg = realPacket->GetStringFromData();
			std::cout << name << " received message: " << msg << std::endl;
		}
	}

protected:
	string name;
};

void TestNetworking() {
	NetworkBase::Initialise();

	string name;
	std::cout << "Please Enter a Username: ";
	std::cin >> name;

	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver(name);

	int port = NetworkBase::GetDefaultPort();

	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();

	server->RegisterPacketHandler(String, &serverReceiver);
	client->RegisterPacketHandler(String, &clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);

	for(int i = 0; i < 100; ++i)
	{
		server->SendGlobalMessage(StringPacket("Server says hello! " + std::to_string(i)));
		client->SendPacket(StringPacket(clientReceiver.GetName() + " says ey up! " + std::to_string(i)));

		server->UpdateServer();
		client->UpdateClient();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	NetworkBase::Destroy();
}

vector<Vector3> testNodes;

void TestPathfinding() {
	NavigationGrid grid("TestGrid1.txt");

	NavigationPath outPath;

	Vector3 startPos(80, 0, 80);
	Vector3 endPos(80, 0, 10);


	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos))
	{
		testNodes.push_back(pos);
	}
}

void DisplayPathfinding() {
	for(int i = 1; i < testNodes.size(); ++i)
	{
		Vector3 a = testNodes[i - 1]*2;
		Vector3 b = testNodes[i]*2;

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}



/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
	Window*w = Window::CreateGameWindow("Golf wit' Lads", 1280, 720);

	if (!w->HasInitialised()) {
		return -1;
	}	
	//TestStateMachine();
	//TestNetworking();
	//TestPathfinding();
	
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	TutorialGame* g = new TutorialGame();
	//NetworkGame* n = new NetworkGame();
	//GolfGame* g = new GolfGame();
	

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDelta() / 1000.0f;

		if (dt > 1.0f) {
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_NEXT)) {
			w->ShowConsole(false);
		}

		//DisplayPathfinding();

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		//n->UpdateServer(dt);
		g->UpdateGame(dt);
		
	}
	Window::DestroyGameWindow();
}

