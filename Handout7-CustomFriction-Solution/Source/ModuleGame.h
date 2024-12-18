#pragma once

#include "Globals.h"
#include "Module.h"

#include "raylib.h"
#include <vector>
#include "Timer.h"

class PhysBody;
class PhysicEntity;
class b2World;
class b2Body;
class b2BodyDef;
class b2PolygonShape;
class b2Vec2;

enum {
	TDC_LEFT = 0x1,
	TDC_RIGHT = 0x2,
	TDC_UP = 0x4,
	TDC_DOWN = 0x8
};

class Circle
{
public:
	Circle(PhysBody* i_body, float i_mass);
	~Circle();

	float GetLifeTime() const;
	void Draw();
	void Update(float i_staticFricion, float i_dynamicFriction);

private:
	PhysBody* m_body = nullptr;
	Timer m_lifeTime;
	float mass;

};

class TDTire 
{
public:
	b2Body* m_body;

	TDTire(b2World* world) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		m_body = world->CreateBody(&bodyDef);

		b2PolygonShape polygonShape;
		polygonShape.SetAsBox(0.5f, 1.25f);
		m_body->CreateFixture(&polygonShape, 1);//shape, density

		m_body->SetUserData(this);
	}

	~TDTire() {
		m_body->GetWorld()->DestroyBody(m_body);
	}
private:
	b2Vec2 GetLateralVelocity();
	b2Vec2 GetForwardVelocity();
	void UpdateFriction(int controlState);
	int m_controlState = 0;

	void Keyboard(unsigned char key);
	void KeyboardUp(unsigned char key);
};


class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	bool Start();
	update_status Update();
	bool CleanUp();

public:

	
	// TODO 1: Every second, create a circle at the leftmost side of the screen at a random height. These circles should be destroyed after 10 seconds.
	// TIP: You can check (and maybe reuse) some previous Handout...
	Timer m_creationTimer;
	std::vector<Circle> m_circles;

	// TODO 5 & 6:
	std::vector<float> m_staticFrictions = { 0.0f, 0.1f, 0.3f, 0.5f };
	std::vector<float> m_dynamicFrictions = { 0.0f, 0.1f, 0.3f, 0.5f };
	int m_currentStaticFriction = 0;
	int m_currentDynamicFriction = 0;
	Texture2D background;
};

