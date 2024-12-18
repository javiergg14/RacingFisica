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

	float mass = 2;
	m_creationTimer.Start();
	int y = 50;
	PhysBody* circleBody = App->physics->CreateCircle(0, y, 10 * std::log(mass));
	m_circles.emplace_back(std::move(circleBody), mass);
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

	return UPDATE_CONTINUE;
}


Circle::Circle(PhysBody* i_body, float i_mass)
	: m_body(i_body)
	, mass(i_mass)
{
	m_lifeTime.Start();
}




Circle::~Circle()
{
}

float Circle::GetLifeTime() const
{
	return m_lifeTime.ReadSec();
}

void Circle::Draw()
{
	b2Vec2 pos = m_body->body->GetPosition();
	DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), (float)METERS_TO_PIXELS(std::log(mass)), Color{ 128, 0, 0, 128 });

}

b2Vec2 TDTire::GetLateralVelocity()
{
    b2Vec2 currentRightNormal = m_body->GetWorldVector(b2Vec2(1, 0));
    return b2Dot(currentRightNormal, m_body->GetLinearVelocity()) * currentRightNormal;
}

b2Vec2 TDTire::GetForwardVelocity()
{
    b2Vec2 currentRightNormal = m_body->GetWorldVector(b2Vec2(0, 1));
    return b2Dot(currentRightNormal, m_body->GetLinearVelocity()) * currentRightNormal;
}

void TDTire::Keyboard(unsigned char key) {
    switch (key) {
    case 'a': m_controlState |= TDC_LEFT;  break;
    case 'd': m_controlState |= TDC_RIGHT; break;
    case 'w': m_controlState |= TDC_UP;    break;
    case 's': m_controlState |= TDC_DOWN;  break;
    default: Keyboard(key);
    }
}
void TDTire::KeyboardUp(unsigned char key) {
    switch (key) {
    case 'a': m_controlState &= ~TDC_LEFT;  break;
    case 'd': m_controlState &= ~TDC_RIGHT; break;
    case 'w': m_controlState &= ~TDC_UP;    break;
    case 's': m_controlState &= ~TDC_DOWN;  break;
    default: Keyboard(key);
    }
}

void TDTire::UpdateFriction(controlState) 
{
    b2Vec2 impulse = m_body->GetMass() * -GetLateralVelocity();
    m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);
    m_body->ApplyAngularImpulse(0.1f * m_body->GetInertia() * -m_body->GetAngularVelocity(), true);
    b2Vec2 currentForwardNormal = GetForwardVelocity();
    float currentForwardSpeed = currentForwardNormal.Normalize();
    float dragForceMagnitude = -2 * currentForwardSpeed;
    m_body->ApplyForce(dragForceMagnitude * currentForwardNormal, m_body->GetWorldCenter());
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


	