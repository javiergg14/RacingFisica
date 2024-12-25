#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

#include <cmath>
#include <format>

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{

}

ModuleGame::~ModuleGame()
{}

// Load assets
bool ModuleGame::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

    background = LoadTexture("Assets/laceHolderEscenario.png");

	m_creationTimer.Start();

	mass = 1.5f;
	m_creationTimer.Start();
	//int y = 50; //Radio circulo
	/*PhysBody* circleBody = App->physics->CreateCircle(0, y, 10 * std::log(mass));*/
    PhysBody* car = App->physics->CreateRectangle(500, 500, 10 * std::log(mass), 10 * std::log(mass), b2_dynamicBody);
	/*m_circles.emplace_back(std::move(circleBody), mass);*/
    m_tdTire.emplace_back(std::move(car), mass);

	return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update: draw background
update_status ModuleGame::Update()
{
	// TODO 1:
	//if (m_creationTimer.ReadSec() > 1.0f)
	//{
	//	
	//}

	// TODO 5: With each left click, increase the STATIC friction coeficient. (At some point, reset it back to zero). Display it at the bottom of the screen.
	//if (IsMouseButtonPressed(0))
	//{
	//	m_currentStaticFriction = (m_currentStaticFriction + 1) % m_staticFrictions.size();
	//}

	//// TODO 6: With each right click, increase the DYNAMIC friction coeficient. (At some point, reset it back to zero). Display it at the bottom of the screen.
	//if (IsMouseButtonPressed(1))
	//{
	//	m_currentDynamicFriction = (m_currentDynamicFriction + 1) % m_dynamicFrictions.size();
	//}



	//DrawText(std::format("Static Friction: {}/ Dynamic Friction: {}", m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]).c_str(), 10, 600, 30, BLACK);

    DrawTexture(background, 0, 0, WHITE);
    
    for (Circle& c : m_circles)
	{
		c.Update(m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]);
	}

	for (Circle& c : m_circles)
	{
		c.Draw();
	}

    for (Car& c : m_tdTire)
    {
        c.Update(m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]);
    }

    for (Car& c : m_tdTire)
    {
        c.Draw();
    }

	return UPDATE_CONTINUE;
}


Circle::Circle(PhysBody* i_body, float i_mass)
	: m_body(i_body)
	, mass(i_mass)
{
	m_lifeTime.Start();
}

Car::Car(PhysBody* i_body, float i_mass)
    : m_body(i_body)
    , mass(i_mass)
{
    m_lifeTime.Start();
    forceX = 0.3f;  // Fuerza para el movimiento en el eje X (control de rotación)
    forceY = 0.1f;  // Fuerza para el movimiento hacia adelante en el eje Y (automático)
    maxSpeed = 0.7f; // Velocidad máxima permitida
    normalForce = mass * 9.8f;
}




Circle::~Circle()
{
}

Car::~Car()
{
}

float Circle::GetLifeTime() const
{
	return m_lifeTime.ReadSec();
}

float Car::GetLifeTime() const
{
    return m_lifeTime.ReadSec();
}

void Circle::Draw()
{
	b2Vec2 pos = m_body->body->GetPosition();
	DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), (float)METERS_TO_PIXELS(std::log(mass)), Color{ 128, 0, 0, 128 });

}

void Car::Draw()
{
    b2Vec2 pos = m_body->body->GetPosition();
    DrawRectangle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), (float)METERS_TO_PIXELS(std::log(mass)), (float)METERS_TO_PIXELS(std::log(mass)), Color{ 0, 0, 0, 128 });

}

void Circle::Update(float i_staticFricion, float i_dynamicFriction)
{
    float forceX = 0.3f; // Fuerza para el movimiento en el eje X
    float forceY = 0.3f; // Fuerza para el movimiento en el eje Y
    float maxSpeed = 6.0f; // Velocidad máxima permitida

    // Cálculo de fricción estática o dinámica
    if (m_body->body->GetLinearVelocity().LengthSquared() < 0.001f)
    {
        float N = mass * 9.8f;
        float staticFriction = N * i_staticFricion;
        forceX = std::max(0.0f, forceX - staticFriction);
        forceY = std::max(0.0f, forceY - staticFriction);  // Aplicar fricción en el eje Y también
    }
    else
    {
        float N = mass * 9.8f;
        float dynamicFriction = N * i_dynamicFriction;
        forceX = std::max(0.0f, forceX - dynamicFriction);
        forceY = std::max(0.0f, forceY - dynamicFriction);  // Aplicar fricción en el eje Y también
    }

    // Aplicar fuerzas para mover el círculo
    if (IsKeyDown(KEY_D))
    {
        m_body->body->ApplyForce(b2Vec2(forceX, 0.0f), b2Vec2_zero, true);
    }
    if (IsKeyDown(KEY_A))
    {
        m_body->body->ApplyForce(b2Vec2(-forceX, 0.0f), b2Vec2_zero, true);
    }
    if (IsKeyDown(KEY_W))
    {
        m_body->body->ApplyForce(b2Vec2(0.0f, -forceY), b2Vec2_zero, true); // Movimiento hacia arriba
    }
    if (IsKeyDown(KEY_S))
    {
        m_body->body->ApplyForce(b2Vec2(0.0f, forceY), b2Vec2_zero, true); // Movimiento hacia abajo
    }

    // Obtener la velocidad actual del cuerpo
    b2Vec2 velocity = m_body->body->GetLinearVelocity();

    // Limitar la velocidad máxima en ambas direcciones (X y Y)
    if (velocity.Length() > maxSpeed)
    {
        // Normalizar la velocidad y aplicarle la velocidad máxima
        velocity.Normalize();
        velocity *= maxSpeed;

        m_body->body->SetLinearVelocity(velocity);
    }

    // Frenar el movimiento cuando se sueltan las teclas
    if (!IsKeyDown(KEY_D))
    {
        if (velocity.x > 0.0f) // Frenar solo si se mueve hacia la derecha
        {
            m_body->body->ApplyForce(b2Vec2(-forceX * 1.3f, 0.0f), b2Vec2_zero, true); // Frenado en el eje X
        }
    }
    if (!IsKeyDown(KEY_A))
    {
        if (velocity.x < 0.0f) // Frenar solo si se mueve hacia la izquierda
        {
            m_body->body->ApplyForce(b2Vec2(forceX * 1.3f, 0.0f), b2Vec2_zero, true); // Frenado en el eje X
        }
    }
    if (!IsKeyDown(KEY_W))
    {
        if (velocity.y < 0.0f) // Frenar solo si se mueve hacia abajo
        {
            m_body->body->ApplyForce(b2Vec2(0.0f, forceY * 1.3f), b2Vec2_zero, true); // Frenado en el eje Y
        }
    }
    if (!IsKeyDown(KEY_S))
    {
        if (velocity.y > 0.0f) // Frenar solo si se mueve hacia arriba
        {
            m_body->body->ApplyForce(b2Vec2(0.0f, -forceY * 1.3f), b2Vec2_zero, true); // Frenado en el eje Y
        }
    }
}

void Car::Update(float i_staticFricion, float i_dynamicFriction)
{
    // Cálculo de fricción estática o dinámica
    if (m_body->body->GetLinearVelocity().LengthSquared() < 0.001f)
    {
        float staticFriction = normalForce * i_staticFricion;
        forceX = std::max(0.0f, forceX - staticFriction);
        forceY = std::max(0.0f, forceY - staticFriction); // Aplicar fricción en el eje Y también
    }
    else
    {
        float dynamicFriction = normalForce * i_dynamicFriction;
        forceX = std::max(0.0f, forceX - dynamicFriction);
        forceY = std::max(0.0f, forceY - dynamicFriction); // Aplicar fricción en el eje Y también
    }

    // Movimiento automático hacia adelante (en el eje Y)
    m_body->body->ApplyForce(b2Vec2(0.0f, -forceY), b2Vec2_zero, true);

    // Control de rotación
    if (IsKeyDown(KEY_D)) {
        // Girar a la derecha
        m_body->body->SetAngularVelocity(-10.0f);
        printf("%f\n", m_body->body->GetAngle());
    }
    if (IsKeyDown(KEY_A)) {
        // Girar a la izquierda
        m_body->body->SetAngularVelocity(10.0f);
        printf("%f\n", m_body->body->GetAngle());
    }

    b2Vec2 velocity = m_body->body->GetLinearVelocity();

    // Limitar la velocidad máxima en ambas direcciones (X y Y)
    if (velocity.Length() > maxSpeed)
    {
        // Normalizar la velocidad y aplicarle la velocidad máxima
        velocity.Normalize();
        velocity *= maxSpeed;

        m_body->body->SetLinearVelocity(velocity);
    }
}


	