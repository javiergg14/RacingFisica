#pragma once

#include "Module.h"
#include "Globals.h"

#include <vector>   // Para std::vector
#include <cmath>    // Para floor()
#include "Box2D/Box2D.h" // Ruta relativa correcta para Box2D

#define GRAVITY_X 0.0f
#define GRAVITY_Y -0.0f
#define PIXELS_PER_METER 50.0f // if touched change METER_PER_PIXEL too
#define METER_PER_PIXEL 0.02f // this is 1 / PIXELS_PER_METER !
#define METERS_TO_PIXELS(m) ((int) floor(PIXELS_PER_METER * m))
#define PIXEL_TO_METERS(p)  ((float) METER_PER_PIXEL * p)

// Small class to return to other modules to track position and rotation of physics bodies
class PhysBody
{
public:
    PhysBody() : listener(NULL), body(NULL), isActive(false) 
    {}

    void GetPhysicPosition(int& x, int& y) const;
    float GetRotation() const;
    bool Contains(int x, int y) const;
    int RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const;
    void GetPosition(int& x, int& y) const;
    std::vector<b2Vec2> GetVertices() const; // Método bien declarado

public:
    bool isActive; // Estado del sensor
    int width, height;
    b2Body* body;
    Module* listener;
};

// Module --------------------------------------
class ModulePhysics : public Module, public b2ContactListener
{
public:
    ModulePhysics(Application* app, bool start_enabled = true);
    ~ModulePhysics();
    void DestroyBody(b2Body* body) {
        world->DestroyBody(body);
    }
    bool Start();
    update_status PreUpdate();
    virtual update_status PostUpdate() override;
    bool CleanUp();

    PhysBody* CreateCircle(int x, int y, int radius);
    PhysBody* CreateRectangle(int x, int y, int width, int height, b2BodyType Type);
    PhysBody* CreateChain(int x, int y, const int* points, int size);

    void BeginContact(b2Contact* contact);

    PhysBody* CreateRectangleSensor(int x, int y, int width, int height);
    b2World* world; // Cambiar a público
private:
    bool debug;
};
