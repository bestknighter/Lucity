#include "StageState.h"
#include "Game.h"
#include "GameData.h"
#include "InputManager.h"
#include "Camera.h"
#include "Collision.h"

#include "PauseState.h"
#include "EndState.h"

#include "Sprite.h"
#include "Text.h"
#include "HUD.h"
#include "TileMap.h"
#include "Collider.h"
#include "CameraFollower.h"
#include "MainObject.h"

#include "Player.h"
#include "NPC.h"
#include "Monster.h"

#include <algorithm>
#include "config.h"

bool SortRenderOrder_MS(std::shared_ptr<GameObject> i, std::shared_ptr<GameObject> j) {
	return (i->box.y < j->box.y);
}

StageState::StageState() : State() {
	gameOver = false;

	GameData::popAgain = false;

	GameObject* go;

	NPCList.emplace_back(Personality("girl", 150, 200, 1, 3, 1, 3, {"hobo"}, {"suit", "tree"}));
	NPCList.emplace_back(Personality("hobo", 150, 150, 2, 2, 1, 2, {}, {"suit", "trashcan"}));
	NPCList.emplace_back(Personality("luv", 150, 50, 3, 1, 2, 2, {"old"}, {"girl", "suit"}));
	NPCList.emplace_back(Personality("old", 50, 250, 3, 1, 1.5, 2, {}, {"luv", "bench"}));
	NPCList.emplace_back(Personality("suit", 150, 150, 2, 2, 0.5, 1, {}, {}));

	monsterList.emplace_back(Personality("girl", 50, 200, 0.3, 5, 1, 1, {"luv"}, {"hobo"}));
	monsterList.emplace_back(Personality("hobo", 250, 50, 1, 3, 3, 1, {}, {"girl", "luv", "old", "suit"}));
	monsterList.emplace_back(Personality("luv", 150, 150, 10, 2, 1, 2, {}, {"old", "tree"}));
	monsterList.emplace_back(Personality("old", 300, 250, 1, 2, 2, 2, {"tree"}, {"trashcan"}));
	monsterList.emplace_back(Personality("suit", 150, 4000, 5, 0.3, 5, 1, {"bench"}, {}));

	//TileMap
	go = new GameObject();
	set = new TileSet(*go, ASSETS_PATH("/img/tileSet.png"), 64, 64);
	TileMap* map = new TileMap(*go, set, ASSETS_PATH("/map/tileMap.txt") );
	go->AddComponent(map);
	go->box.SetSize(Vec2());
	AddObject(go, "BG");

	int mw = 64*map->GetWidth();
	int mh = 64*map->GetHeight();
	GameData::mapSize = Vec2(mw, mh);
	GameData::upperLimit = 0;

	//Event Countdown
	GameData::eventCD = 60;

	//HUD
	go = new GameObject();
	go->AddComponent(new HUD(*go));
	go->AddComponent(new CameraFollower(*go, Vec2(0, 0)));
	AddObject(go, "GUI");
	
	//Bench
	go = new GameObject();
	go->AddComponentAsFirst(new MainObject(*go, "bench", 1, Vec2(3, 3), true, true));
	go->box.SetPos(Vec2(832, 224));
	AddObject(go, "MAIN");


	go = new GameObject();
	go->AddComponentAsFirst(new MainObject(*go, "bench", 1, Vec2(3, 3), true, true));
	go->box.SetPos(Vec2(320,416));
	AddObject(go, "MAIN");


	go = new GameObject();
	go->AddComponentAsFirst(new MainObject(*go, "bench", 1, Vec2(3, 3), true, true));
	go->box.SetPos(Vec2(960,864));
	AddObject(go, "MAIN");

	//Trashcan
	go = new GameObject();
	go->AddComponentAsFirst(new MainObject(*go, "trashcan", 1, Vec2(3, 3), true, true));
	go->box.SetPos(Vec2(672,224));
	AddObject(go, "MAIN");

	go = new GameObject();
	go->AddComponentAsFirst(new MainObject(*go, "trashcan", 1, Vec2(3, 3), true, true));
	go->box.SetPos(Vec2(352,864));
	AddObject(go, "MAIN");

	go = new GameObject();
	go->AddComponentAsFirst(new MainObject(*go, "trashcan", 1, Vec2(3, 3), true, true));
	go->box.SetPos(Vec2(800,928));
	AddObject(go, "MAIN");

	//Tree
	go = new GameObject();
	go->AddComponentAsFirst(new MainObject(*go, "tree", 1, Vec2(3, 3), true));
	go->box.SetPos(Vec2(224, 224));
	AddObject(go, "MAIN");

	go = new GameObject();
	go->AddComponentAsFirst(new MainObject(*go, "tree", 1, Vec2(3, 3), true));
	go->box.SetPos(Vec2(1056, 224));
	AddObject(go, "MAIN");

	go = new GameObject();
	go->AddComponentAsFirst(new MainObject(*go, "tree", 1, Vec2(3, 3), true));
	go->box.SetPos(Vec2(224, 800));
	AddObject(go, "MAIN");

	//Monsters
	for(int i = 0; i < 5; i++) {
		go = new GameObject();
		go->AddComponentAsFirst(new Monster(*go, monsterList[rand()%monsterList.size()]));
		go->box.SetCenter(rand()%mw, rand()%mh);
		AddObject(go, "MAIN");

		GameData::nMonsters++;
	}
	GameData::nMaxMonsters = GameData::nMonsters;

	//NPCs
	for(int i = 0; i < 10; i++) {
		go = new GameObject();
		go->AddComponentAsFirst(new NPC(*go, NPCList[rand()%NPCList.size()]));
		go->box.SetCenter(rand()%mw, rand()%mh);
		AddObject(go, "MAIN");

		GameData::nCivilians++;
	}
	GameData::nMaxCivilians = GameData::nCivilians;

	//Players
	go = new GameObject();
	go->AddComponentAsFirst(new Player(*go, "lucas", 0));
	go->box.SetCenter(mw/2, mh/2);
	AddObject(go, "MAIN");

	Camera::Follow(go);

	backgroundMusic = Music( ASSETS_PATH("/audio/theme.ogg") );
	backgroundMusic.Play();
}

StageState::~StageState() {
	GameData::mapSize = Vec2();
	GameData::nMonsters = 0;
	GameData::nCivilians = 0;
	GameData::player = std::weak_ptr<GameObject>();
	delete set;
}

void StageState::LoadAssets() {

}

void StageState::Start() {
	LoadAssets();
	StartArray();
	started = true;
}

void StageState::Pause() {
	//Camera::Unfollow();
	backgroundMusic.Stop();
}

void StageState::Resume() {
	GameData::popAgain = false;

	//if(!GameData::player.expired())
		//Camera::Follow(GameData::player.lock().get());

	backgroundMusic.Play();
}

void StageState::CollisionCheck() {
	for(unsigned i = 0; i < objects["MAIN"].size(); i++) {
		for(unsigned j = i+1; j < objects["MAIN"].size(); j++) {
			if(objects["MAIN"][i]->IsActive() && objects["MAIN"][j]->IsActive()) {
				Collider* objA = (Collider*) objects["MAIN"][i]->GetComponent("Collider");
				Collider* objB = (Collider*) objects["MAIN"][j]->GetComponent("Collider");
				if(objA && objB) {
					bool collided = false;
					if(objA->GetMode() == Collider::RECT) {
						if(objB->GetMode() == Collider::RECT)
							collided = Collision::IsCollidingRectRect(objA->box, objB->box, objA->rotation, objB->rotation);
						else
							collided = Collision::IsCollidingCircleRect(objB->circle, objA->box, objA->rotation);
					}
					else {
						if(objB->GetMode() == Collider::RECT)
							collided = Collision::IsCollidingCircleRect(objA->circle, objB->box, objB->rotation);
						else
							collided = Collision::IsCollidingCircleCircle(objA->circle, objB->circle);	
					}
					if(collided) {
						objects["MAIN"][i]->NotifyCollision(*objects["MAIN"][j]);
						objects["MAIN"][j]->NotifyCollision(*objects["MAIN"][i]);
					}
				}
			}
		}
	}
}

void StageState::DeletionCheck() {
	for(auto& i: objects) {
		for(int j = i.second.size()-1; j >= 0; j--) {
			if(i.second[j]->IsDead()) {
				if(Camera::GetFocus() == i.second[j].get())
					Camera::Unfollow();

				if(i.second[j]->GetComponent("Soul")) {
					GameData::nMonsters--;
					GameData::eventT.Restart();
				}
				else if(i.second[j]->GetComponent("NPC") && !i.second[j]->GetComponent("Monster")) {
					GameData::nCivilians--;
				}
				i.second.erase(i.second.begin()+j);
			}
		}
	}
}

void StageState::Update(float dt) {
	quitRequested = InputManager::QuitRequested();
	if(InputManager::KeyPress(ESCAPE_KEY))
		Game::GetInstance().Push(new PauseState());

	if(InputManager::KeyPress(SDLK_F1))
		GameData::debug = !GameData::debug;

	if(InputManager::KeyPress(SDLK_F2))
		GameData::paused = !GameData::paused;

	if(InputManager::KeyPress(SDLK_F5)) {
		gameOver = true;
		GameData::playerVictory = false;
	}

	if(InputManager::KeyPress(SDLK_F6)) {
		gameOver = true;
		GameData::playerVictory = true;
	}

	if(!gameOver) {
		if(!GameData::paused)
			GameData::eventT.Update(dt);
		if(GameData::eventT.Get() > GameData::eventCD) {
			for(unsigned int i = 0; i < objects["MAIN"].size(); i++) {
				Monster* mon = (Monster*) objects["MAIN"][i]->GetComponent("Monster");
				if(mon) {
					if(!mon->IsTransformed()) {
						mon->Transform();
						GameData::eventT.Restart();
						break;
					}
				}
			}
		}

		if(GameData::nMonsters == 0) {
			gameOver = true;
			GameData::playerVictory = true;
			GameData::bossStageUnlocked = true;
		}
		else if(GameData::player.expired()) {
			gameOver = true;
			GameData::playerVictory = false;
		}
	}
	else {
		Game::GetInstance().Push(new EndState());
	}

	UpdateArray(dt, "BG");
	UpdateArray(dt, "EFFECT");
	UpdateArray(dt, "MAIN");
	UpdateArray(dt, "MISC");
	UpdateArray(dt, "GUI");
	CollisionCheck();
	DeletionCheck();

	std::sort(objects["EFFECT"].begin(), objects["EFFECT"].end(), SortRenderOrder_MS);
	std::sort(objects["MAIN"].begin(), objects["MAIN"].end(), SortRenderOrder_MS);
}

void StageState::Render() {
	RenderArray("BG");
	RenderArray("EFFECT");
	RenderArray("MAIN");
	RenderArray("MISC");
	RenderArray("GUI");
}
