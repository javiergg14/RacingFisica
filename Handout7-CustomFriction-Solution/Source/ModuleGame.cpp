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
bool ModuleGame::Start()
{
    LOG("Loading Intro assets");
    bool ret = true;

    background = LoadTexture("Assets/laceHolderEscenario.png");

	mass = 1.5f;

	m_creationTimer.Start();
	//int y = 50; //Radio circulo
	//PhysBody* circleBody = App->physics->CreateCircle(0, y, 10 * std::log(mass));
    PhysBody* car = App->physics->CreateRectangle(500, 500, 10 * std::log(mass), 10 * std::log(mass), b2_dynamicBody);
	/*m_circles.emplace_back(std::move(circleBody), mass);*/
    m_tdTire.emplace_back(std::move(car), mass);
    lapCount = 0;
    currentCheckpointIndex = 0;

    CreateCheckpoints();
    car->listener = this;
    for (PhysBody* checkpoint : checkpoints)
    {
        checkpoint->listener = this;
    }
    m_currentStaticFriction = (m_currentStaticFriction + 2);
    //// TODO 6: With each right click, increase the DYNAMIC friction coeficient. (At some point, reset it back to zero). Display it at the bottom of the screen.
    m_currentDynamicFriction = (m_currentDynamicFriction + 2);

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
    LOG("Collision detected");

    // Verifica si el coche toca un checkpoint
    if (bodyA == car || bodyB == car)
    {
        PhysBody* checkpoint = checkpoints[currentCheckpointIndex];

        if (checkpoint == nullptr) {
            LOG("Checkpoint is NULL!"); // Verifica si el checkpoint es NULL
            return; // Salir si el checkpoint es NULL
        }

        // Verifica si se ha alcanzado el checkpoint correcto
        if (checkpoint == checkpoints[currentCheckpointIndex])
        {
            LOG("Checkpoint %d reached!", currentCheckpointIndex + 1);
            currentCheckpointIndex++;

            // Verifica si se ha pasado el último checkpoint
            if (currentCheckpointIndex >= checkpoints.size())
            {
                // Verifica si es el checkpoint de inicio/fin
                if (checkpoint == checkpoints[0]) // Checkpoint de inicio/fin
                {
                    lapCount++; // Incrementar vueltas
                    LOG("Lap completed! Total laps: %d", lapCount);
                }
                currentCheckpointIndex = 0; // Reiniciar al primer checkpoint
            }
        }
        else
        {
            LOG("Checkpoint reached is not the current checkpoint.");
        }
    }
}
// Update: draw background
// En el método Update
update_status ModuleGame::Update()
{
    DrawTexture(background, 0, 0, WHITE);
    DrawText(TextFormat("Laps: %d", lapCount), 20, 20, 30, WHITE);
    DrawText(std::format("Static Friction: {}/ Dynamic Friction: {}", m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]).c_str(), 300, 600, 30, WHITE);

    // Manejo del turbo
    if (turboActive) {
        // Si el turbo está activo, disminuir el tiempo usado
        turboUsedTime += GetFrameTime();
        if (turboUsedTime >= turboDuration) {
            turboActive = false; // Desactivar el turbo si se ha alcanzado el tiempo máximo
            turboUsedTime = turboDuration; // Asegurarse de que no exceda la duración
        }
    }
    else {
        // Recarga del turbo
        if (turboRechargeTimer < turboRechargeDuration) {
            turboRechargeTimer += GetFrameTime(); // Recarga el turbo
            if (turboRechargeTimer > turboRechargeDuration) {
                turboRechargeTimer = turboRechargeDuration; // Limitar a la duración máxima
            }
        }
        // Retrocede la barra de turbo gastado
        if (turboUsedTime > 0.0f) {
            turboUsedTime -= GetFrameTime() * (turboDuration / turboRechargeDuration); // Ajusta la velocidad de recarga
            if (turboUsedTime < 0.0f) {
                turboUsedTime = 0.0f; // No permitir que el tiempo usado sea negativo
            }
        }
    }
    // Activar el turbo si se presiona la tecla y hay tiempo disponible
    if (IsKeyPressed(KEY_LEFT_SHIFT) && turboRechargeTimer >= 0.0f) {
        turboActive = true;
        turboRechargeTimer -= GetFrameTime(); // Disminuir el tiempo de recarga
        if (turboRechargeTimer < 0.0f) {
            turboRechargeTimer = 0.0f; // No permitir que el temporizador sea negativo
        }
    }
    // Desactiva el turbo si se suelta la tecla Shift
    if (IsKeyReleased(KEY_LEFT_SHIFT)) {
        turboActive = false; // Desactivar el turbo al soltar la tecla
    }
    // Aplica el turbo a la velocidad del coche
    for (Car& c : m_tdTire) {
        if (turboActive) {
            c.ApplyTurbo(); // Método que debes implementar en la clase Car
        }
        c.Update(m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]);
    }
    // Dibuja el coche y otros elementos
    for (Circle& c : m_circles) {
        c.Update(m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]);
        c.Draw();
    }
    for (Car& c : m_tdTire) {
        c.Draw();
    }
    // Dibuja la barra de uso
    float barWidth = 200.0f; // Ancho de la barra
    float barHeight = 20.0f; // Altura de la barra
    float usedPercentage = turboUsedTime / turboDuration; // Porcentaje de uso
    // Dibuja la barra de uso
    DrawRectangle(300, 100, barWidth, barHeight, BLUE); // Fondo de la barra
    DrawRectangle(300, 100, barWidth * (1.0f - usedPercentage), barHeight, LIGHTGRAY); // Barra de turbo usada

    if (turboActive) {
        DrawText("Turbo Active!", 300, 50, 20, GREEN);
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
}
void Car::ApplyTurbo() {
    float turboForce = 1.0f; // Ajusta la fuerza del turbo según sea necesario
    b2Vec2 velocity = m_body->body->GetLinearVelocity();

    // Normaliza la dirección de la velocidad
    if (velocity.Length() > 0) {
        velocity.Normalize();
        // Aplica la fuerza en la dirección de la velocidad
        b2Vec2 force(velocity.x * turboForce, velocity.y * turboForce); // Crea un nuevo b2Vec2 con la fuerza
        m_body->body->ApplyForce(force, m_body->body->GetWorldCenter(), true);
    }
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
    forceX = 10.0f;  // Fuerza para el movimiento en el eje X (control de rotación)
    forceY = 10.0f;  // Fuerza para el movimiento hacia adelante en el eje Y (automático)
    maxSpeed = 0.001f; // Velocidad máxima permitida
    normalForce = mass * 9.8f;
    framesWithoutInput = 0;
    maxFramesWithoutInput = 240;

    // Obtener la velocidad actual del cuerpo
    b2Vec2 velocity = m_body->body->GetLinearVelocity();

    b2Vec2 force = b2Vec2(0,0);

    // Cálculo de fricción estática o dinámica
    if (velocity.Length() < 0.001f)
    {
        float staticFriction = normalForce * i_staticFricion;
        forceX = std::max(0.0f, forceX - staticFriction);
        forceY = std::max(0.0f, forceY - staticFriction);
    }

    // Aplicar fuerzas para mover el círculo
    float dynamicFriction = normalForce * i_dynamicFriction;
    
    if (IsKeyDown(KEY_D))
    {
        force.x = forceX;
    }
    if (IsKeyDown(KEY_A))
    {
        force.x = -forceX;
    }
    if (IsKeyDown(KEY_W))
    {
        force.y = -forceY;
    }
    if (IsKeyDown(KEY_S))
    {
        force.y = forceY;
    }
    
    m_body->body->ApplyForce(force, b2Vec2_zero, true); 

    if (velocity.Length() > 0)
    {
        printf("ENTRAA");
        velocity.x -= std::copysign(dynamicFriction, velocity.x);
        velocity.y -= std::copysign(dynamicFriction, velocity.y);
    }

    //if (velocity.Length() > maxSpeed)
    //{
    //    float velocityGap = velocity.Length() - maxSpeed;
    //    velocity.x -= std::copysign(velocityGap / 2, velocity.x);
    //    velocity.y -= std::copysign(velocityGap / 2, velocity.y);
    //}

    m_body->body->SetLinearVelocity(velocity);
}

void ModuleGame::CreateCheckpoints()
{
    // Checkpoint 1: Inicio/Fin
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 652, 75, 10));  // Checkpoint 1: Inicio/Fin 

    // Otros checkpoints
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 550, 75, 10));  // Checkpoint 2: Carretera derecha
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 400, 75, 10));  // Checkpoint 3: Carretera derecha
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 250, 75, 10));  // Checkpoint 4: Carretera derecha
    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 100, 75, 10));  // Checkpoint 5: Carretera derecha

    checkpoints.push_back(App->physics->CreateRectangleSensor(850, 57, 10, 65));  // Checkpoint 6: Carretera de Arriba
    checkpoints.push_back(App->physics->CreateRectangleSensor(700, 57, 10, 65));  // Checkpoint 7: Carretera de Arriba
    checkpoints.push_back(App->physics->CreateRectangleSensor(550, 57, 10, 65));  // Checkpoint 8: Carretera de Arriba
    checkpoints.push_back(App->physics->CreateRectangleSensor(400, 57, 10, 65));  // Checkpoint 9: Carretera de Arriba
    checkpoints.push_back(App->physics->CreateRectangleSensor(250, 57, 10, 65));  // Checkpoint 10: Carretera de Arriba

    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 150, 100, 10));  // Checkpoint 11: Carretera izquierda 
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 300, 100, 10));  // Checkpoint 12: Carretera izquierda
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 400, 100, 10));  // Checkpoint 13: Carretera izquierda
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 550, 100, 10));  // Checkpoint 14: Carretera izquierda
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 650, 100, 10));  // Checkpoint 15: Carretera izquierda
    checkpoints.push_back(App->physics->CreateRectangleSensor(180, 800, 175, 10));  // Checkpoint 16: Carretera izquierda

    checkpoints.push_back(App->physics->CreateRectangleSensor(275, 907, 10, 125));  // Checkpoint 17: Carretera izquierda Curva Abajo
    checkpoints.push_back(App->physics->CreateRectangleSensor(425, 907, 10, 125));  // Checkpoint 18: Carretera izquierda Curva Abajo

    checkpoints.push_back(App->physics->CreateRectangleSensor(465, 755, 75, 10));  // Checkpoint 19: Saltos Carril Izquierdo
    checkpoints.push_back(App->physics->CreateRectangleSensor(465, 628, 75, 10));  // Checkpoint 20: Saltos Carril Izquierdo
    checkpoints.push_back(App->physics->CreateRectangleSensor(465, 500, 75, 10));  // Checkpoint 21: Saltos Carril izquierdo
    checkpoints.push_back(App->physics->CreateRectangleSensor(465, 400, 75, 10));  // Checkpoint 22: Saltos Carril Izquierdo

    checkpoints.push_back(App->physics->CreateRectangleSensor(570, 285, 10, 140));  // Checkpoint 23: Curva Mid-Saltos

    checkpoints.push_back(App->physics->CreateRectangleSensor(685, 400, 75, 10));  // Checkpoint 24: Saltos Carril Derecho
    checkpoints.push_back(App->physics->CreateRectangleSensor(685, 508, 75, 10));  // Checkpoint 25: Saltos Carril Derecho
    checkpoints.push_back(App->physics->CreateRectangleSensor(685, 635, 75, 10));  // Checkpoint 26: Saltos Carril Derecho
    checkpoints.push_back(App->physics->CreateRectangleSensor(685, 765, 75, 10));  // Checkpoint 27: Saltos Carril Derecho

    checkpoints.push_back(App->physics->CreateRectangleSensor(735, 945, 10, 95));  // Checkpoint 28: Ultima Curva
    checkpoints.push_back(App->physics->CreateRectangleSensor(880, 945, 10, 95));  // Checkpoint 29: Ultima curva

    checkpoints.push_back(App->physics->CreateRectangleSensor(928, 800, 75, 10));  // Checkpoint 30: Carretera derecha

    checkpoints.push_back(checkpoints[0]);
}