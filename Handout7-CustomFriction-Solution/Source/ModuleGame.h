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
class ModuleGame;
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
    // Modificación en el constructor para incluir una referencia o puntero a ModuleGame
    Car(PhysBody* i_body, float i_mass, int i_player, ModuleGame* moduleGame);

    void ApplyTurbo();
    ~Car();
    float GetLifeTime() const;
    int GetPlayer();
    void Draw(Texture2D texture);
    void Update(float i_staticFriction, float i_dynamicFriction);
    PhysBody* GetBody();
    Module* module = nullptr;

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

    // Puntero a ModuleGame
    ModuleGame* m_moduleGame = nullptr;
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

    //Sounds
    Sound Selection;
    Sound SwitchOption;
    // Estado del juego
    bool isMenuActive = true;
    bool isMapSelectorActive = false;
    bool inCredits = false;
    int selectedMenuOption = 0;
    int selectedMapIndex = 0;
    bool showCredits = false;
    bool isEnterPressed = false;
    bool showControls = false;
// Turbo para car1
    bool car1TurboActive = false;
    float car1TurboUsedTime = 0.0f;
    float car1TurboRechargeTimer = 0.0f;

    // Turbo para car2
    bool car2TurboActive = false;
    float car2TurboUsedTime = 0.0f;
    float car2TurboRechargeTimer = 0.0f;

    // Constantes compartidas
    const float turboDuration = 2.0f; // Duración del turbo en segundos
    const float turboRechargeDuration = 7.0f; // Tiempo de recarga en segundos

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
    Texture2D controlsTexture;
    float mass;

    Car* car1;
    Car* car2;
};
