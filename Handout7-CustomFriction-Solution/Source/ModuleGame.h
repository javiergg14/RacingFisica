#pragma once

#include "Globals.h"
#include "Module.h"

#include "raylib.h"
#include <vector>
#include "Timer.h"

// Forward declarations
class PhysBody;
class PhysicEntity;
class b2World;

// Enums
enum {
    TDC_LEFT = 0x1,
    TDC_RIGHT = 0x2,
    TDC_UP = 0x4,
    TDC_DOWN = 0x8
};

// Circle class
class Circle
{
public:
    Circle(PhysBody* i_body, float i_mass);
    ~Circle();

    float GetLifeTime() const;
    void Draw();
    void Update(float i_staticFriction, float i_dynamicFriction);

private:
    PhysBody* m_body = nullptr;
    Timer m_lifeTime;
    float mass;
};

// Car class
class Car
{
public:
    Car(PhysBody* i_body, float i_mass, int i_player);
    void ApplyTurbo();
    ~Car();

    float GetLifeTime() const;
    int GetPlayer();
    void Draw(Texture2D texture);
    void Update(float i_staticFriction, float i_dynamicFriction);
    PhysBody* GetBody();

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
    int player;
};

// Collider class
class Collider
{
public:
    Collider(PhysBody* i_body);
    ~Collider();

    void Draw();

private:
    PhysBody* m_body = nullptr;
};


class ModuleGame : public Module
{
public:
    ModuleGame(Application* app, bool start_enabled = true);
    ~ModuleGame();

    bool Start();
    update_status Update();
    void CreateCheckpoints();
    void CreateColliders();
    bool CleanUp();
    bool MainMenu();
    void OnCollision(PhysBody* bodyA, PhysBody* bodyB);

public:
    // Estado del juego
    bool isMenuActive = true;
    bool isMapSelectorActive = false;
    bool inCredits = false;
    int selectedMenuOption = 0;
    int selectedMapIndex = 0;
    bool showCredits = false;
    bool isEnterPressed = false;

    //Variables de Turbo
    float turboRechargeTimer = 0.0f;
    float turboRechargeDuration = 5.0f; //Tiempo para recargar Turbo
    float turboUsedTime = 0.0f;
    float turboDuration = 1.5f;         //Tiempo de Turbo
    bool turboActive = false;

    bool gameFinished = false;
    float totalTime = 0.0f;
    int lapCount = 0;

    // Checkpoints y coche
    std::vector<PhysBody*> checkpoints;
    int currentCheckpointIndex = 0;
    PhysBody* car = nullptr;

    Timer m_creationTimer;
    std::vector<Circle> m_circles;

    std::vector<Car> m_tdTire;

    std::vector<Collider> m_colliders;

    //Fricción
    std::vector<float> m_staticFrictions = { 0.0f, 0.1f, 0.3f, 0.5f };
    std::vector<float> m_dynamicFrictions = { 0.0f, 0.1f, 0.3f, 0.5f };
    int m_currentStaticFriction;
    int m_currentDynamicFriction;

    // Texturas
    Texture2D mapTextures[3];
    Texture2D mapSelectorBgTexture;
    Texture2D mapSelectorTextures[3];
    Texture2D MenuTexture;
    Texture2D creditsTexture;
    Texture2D car1Texture;
    Texture2D car2Texture;

    
    float mass;

    Car* car1;
    Car* car2;
};
