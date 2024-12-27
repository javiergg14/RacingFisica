#pragma once

#include "Globals.h"
#include "Module.h"

#include "raylib.h"
#include <vector>
#include "Timer.h"

class PhysBody;
class PhysicEntity;
class b2World;

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

class Car
{
public:

	Car(PhysBody* i_body, float i_mass);
	void ApplyTurbo();
	~Car();

	float GetLifeTime() const;
	void Draw();
	void Update(float i_staticFricion, float i_dynamicFriction);

private:
	PhysBody* m_body = nullptr;
	Timer m_lifeTime;
	float mass;
	float forceX;
	float forceY;
	float maxSpeed;
	float normalForce;
	float staticFriction;
	int framesWithoutInput;
	int maxFramesWithoutInput;
};


class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	bool Start();
	update_status Update();
	void CreateCheckpoints();
	bool CleanUp();
	void OnCollision(PhysBody* bodyA, PhysBody* bodyB);

public:
public:
	// Variables necesarias
	float turboRechargeTimer = 0.0f; // Temporizador de recarga
	float turboRechargeDuration = 5.0f; // Duración de la recarga
	float turboUsedTime = 0.0f; // Tiempo usado del turbo
	float turboDuration = 1.5f; // Duración del turbo
	bool turboActive = false; // Estado del turbo

	std::vector<PhysBody*> checkpoints;
	int currentCheckpointIndex = 0;    //checkpoint Index
	int lapCount = 0;                  // Contador de vueltas
	PhysBody* car = nullptr;

	// TODO 1: Every second, create a circle at the leftmost side of the screen at a random height. These circles should be destroyed after 10 seconds.
	// TIP: You can check (and maybe reuse) some previous Handout...
	Timer m_creationTimer;
	std::vector<Circle> m_circles;
	std::vector<Car> m_tdTire;

	// TODO 5 & 6:
	std::vector<float> m_staticFrictions = { 0.0f, 0.1f, 0.3f, 0.5f };
	std::vector<float> m_dynamicFrictions = { 0.0f, 0.1f, 0.3f, 0.5f };
	int m_currentStaticFriction = 0;
	int m_currentDynamicFriction = 0;
	Texture2D background;
	float mass;
};

