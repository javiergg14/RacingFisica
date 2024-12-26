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

    // Actualizaci�n de checkpoints
// Sensores como l�neas rectangulares que atraviesan la pista
    CreateCheckpoints();
    for (PhysBody* checkpoint : checkpoints)
    {
        checkpoint->listener = this;
    }

    return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}
// OnCollision para manejar colisiones
void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
    // Verificar si el coche toca un checkpoint
    if (bodyA == car || bodyB == car)
    {
        PhysBody* checkpoint = (bodyA == car) ? bodyB : bodyA;

        if (checkpoint == checkpoints[currentCheckpointIndex])
        {
            currentCheckpointIndex++;
            if (currentCheckpointIndex >= checkpoints.size())
            {
                currentCheckpointIndex = 0; // Reiniciar al primer checkpoint
                lapCount++;                // Incrementar vueltas
                LOG("Lap completed! Total laps: %d", lapCount);
            }
        }
    }
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
	if (IsMouseButtonPressed(0))
	{
		m_currentStaticFriction = (m_currentStaticFriction + 1) % m_staticFrictions.size();
	}

	//// TODO 6: With each right click, increase the DYNAMIC friction coeficient. (At some point, reset it back to zero). Display it at the bottom of the screen.
	if (IsMouseButtonPressed(1))
	{
		m_currentDynamicFriction = (m_currentDynamicFriction + 1) % m_dynamicFrictions.size();
	}



	//DrawText(std::format("Static Friction: {}/ Dynamic Friction: {}", m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]).c_str(), 10, 600, 30, BLACK);

    DrawTexture(background, 0, 0, WHITE);
    DrawText(TextFormat("Laps: %d", lapCount), 20, 20, 30, WHITE);
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
    forceX = 0.3f;  // Fuerza para el movimiento en el eje X (control de rotaci�n)
    forceY = 0.3f;  // Fuerza para el movimiento hacia adelante en el eje Y (autom�tico)
    maxSpeed = 0.7f; // Velocidad m�xima permitida
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
    float maxSpeed = 6.0f; // Velocidad m�xima permitida

    // C�lculo de fricci�n est�tica o din�mica
    if (m_body->body->GetLinearVelocity().LengthSquared() < 0.001f)
    {
        float N = mass * 9.8f;
        float staticFriction = N * i_staticFricion;
        forceX = std::max(0.0f, forceX - staticFriction);
        forceY = std::max(0.0f, forceY - staticFriction);  // Aplicar fricci�n en el eje Y tambi�n
    }
    else
    {
        float N = mass * 9.8f;
        float dynamicFriction = N * i_dynamicFriction;
        forceX = std::max(0.0f, forceX - dynamicFriction);
        forceY = std::max(0.0f, forceY - dynamicFriction);  // Aplicar fricci�n en el eje Y tambi�n
    }

    // Aplicar fuerzas para mover el c�rculo
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

    // Limitar la velocidad m�xima en ambas direcciones (X y Y)
    if (velocity.Length() > maxSpeed)
    {
        // Normalizar la velocidad y aplicarle la velocidad m�xima
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
    // C�lculo de fricci�n est�tica o din�mica
    if (m_body->body->GetLinearVelocity().LengthSquared() < 0.001f)
    {
        float staticFriction = normalForce * i_staticFricion;
        forceX = std::max(0.0f, forceX - staticFriction);
        forceY = std::max(0.0f, forceY - staticFriction);  // Aplicar fricci�n en el eje Y tambi�n
    }
    else
    {
        float dynamicFriction = normalForce * i_dynamicFriction;
        forceX = std::max(0.0f, forceX - dynamicFriction);
        forceY = std::max(0.0f, forceY - dynamicFriction);  // Aplicar fricci�n en el eje Y tambi�n
    }

    // Aplicar fuerzas para mover el c�rculo
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

    // Limitar la velocidad m�xima en ambas direcciones (X y Y)
    if (velocity.Length() > maxSpeed)
    {
        // Normalizar la velocidad y aplicarle la velocidad m�xima
        velocity.Normalize();
        velocity *= maxSpeed;

        m_body->body->SetLinearVelocity(velocity);
    }
}
void ModuleGame::CreateCheckpoints() 
{
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 652, 75, 10));  // Checkpoint 1: Inicio/Fin Carretera derecha
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 550, 75, 10));  // Checkpoint 2: Inicio/Fin Carretera derecha
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 400, 75, 10));  // Checkpoint 3: Inicio/Fin Carretera derecha
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 250, 75, 10));  // Checkpoint 4: Inicio/Fin Carretera derecha
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 100, 75, 10));  // Checkpoint 5: Inicio/Fin Carretera derecha
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 800, 75, 10));  // Checkpoint 7: Inicio/Fin Carretera derecha

    checkpoints.push_back(App->physics->CreateRectangleSensor(735, 945, 10, 95));  // Checkpoint 8: Ultima Curva
    checkpoints.push_back(App->physics->CreateRectangleSensor(880, 945, 10, 95));  // Checkpoint 9: Ultima curva

    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 150, 100, 10));  // Checkpoint 10: Carretera izquierda 
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 300, 100, 10));  // Checkpoint 11: Carretera izquierda
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 400, 100, 10));  // Checkpoint 12: Carretera izquierda
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 550, 100, 10));  // Checkpoint 13: Carretera izquierda
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 650, 100, 10));  // Checkpoint 14: Carretera izquierda
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 800, 175, 10));  // Checkpoint 15: Carretera izquierda

    checkpoints.push_back(App->physics->CreateRectangleSensor(275, 907, 10, 125));  // Checkpoint 16: Carretera izquierda Curva Abajo
    checkpoints.push_back(App->physics->CreateRectangleSensor(425, 907, 10, 125));  // Checkpoint 17: Carretera izquierda Curva Abajo

    checkpoints.push_back(App->physics->CreateRectangleSensor(570, 285, 10, 140));  // Checkpoint 18: Curva Mid-Saltos

    checkpoints.push_back(App->physics->CreateRectangleSensor(250, 57, 10, 65));  // Checkpoint 19: Carretera de Arriba
    checkpoints.push_back(App->physics->CreateRectangleSensor(400, 57, 10, 65));  // Checkpoint 20: Carretera de Arriba
    checkpoints.push_back(App->physics->CreateRectangleSensor(550, 57, 10, 65));  // Checkpoint 21: Carretera de Arriba
    checkpoints.push_back(App->physics->CreateRectangleSensor(700, 57, 10, 65));  // Checkpoint 22: Carretera de Arriba
    checkpoints.push_back(App->physics->CreateRectangleSensor(850, 57, 10, 65));  // Checkpoint 23: Carretera de Arriba

    checkpoints.push_back(App->physics->CreateRectangleSensor(465, 500, 75, 10));  // Checkpoint 24: Saltos Carril izquierdo
    checkpoints.push_back(App->physics->CreateRectangleSensor(465, 400, 75, 10));  // Checkpoint 25: Saltos Carril Izquierdo
    checkpoints.push_back(App->physics->CreateRectangleSensor(465, 628, 75, 10));  // Checkpoint 26: Saltos Carril Derecho
    checkpoints.push_back(App->physics->CreateRectangleSensor(465, 755, 75, 10));  // Checkpoint 27: Saltos Carril Derecho

    checkpoints.push_back(App->physics->CreateRectangleSensor(685, 400, 75, 10));  // Checkpoint 28: Saltos Carril Derecho
    checkpoints.push_back(App->physics->CreateRectangleSensor(685, 508, 75, 10));  // Checkpoint 29: Saltos Carril Derecho
    checkpoints.push_back(App->physics->CreateRectangleSensor(685, 635, 75, 10));  // Checkpoint 30: Saltos Carril Derecho
    checkpoints.push_back(App->physics->CreateRectangleSensor(685, 765, 75, 10));  // Checkpoint 31: Saltos Carril Derecho
}


	