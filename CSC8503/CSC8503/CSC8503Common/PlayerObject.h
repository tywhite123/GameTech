#pragma once
#include "GameObject.h"

using namespace NCL::CSC8503;

class Level;

class PlayerObject :
	public GameObject
{
public:
	PlayerObject(string name);
	virtual ~PlayerObject();

	virtual void OnCollisionBegin(GameObject* otherObject) override;
	//virtual void OnCollisionEnd(GameObject* otherObject) override;

	Level* level;


};


class Level
{
private:
	static Level* instance;
	int level;
	
	Level() { level = 1; }

public:
	static Level* GetInstance()
	{
		if (instance == 0)
			instance = new Level();


		return instance;
	}
	bool loadNext = false;
	void SetLevel(int l) { level = l; }
	int GetLevel() { return level; }
};


