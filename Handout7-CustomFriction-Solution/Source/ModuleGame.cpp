#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

#include <cmath>
#include <format>

ModuleGame::ModuleGame(Application* app, bool start_enabled): Module(app, start_enabled), isMenuActive(true) 
{
}
ModuleGame::~ModuleGame()
{}
bool ModuleGame::Start()
{
    MenuTexture = LoadTexture("Assets/Menu.png");
    background = LoadTexture("Assets/laceHolderEscenario.png");
    creditsTexture = LoadTexture("Assets/Credits.png");
    LOG("Loading Intro assets");
    bool ret = true;

    // Crear el coche en el centro de la pantalla
    mass = 100.0f; // Masa del coche
    car = App->physics->CreateRectangle(800, 650, 50, 100, b2_dynamicBody); // Dimensiones y tipo
    m_tdTire.emplace_back(std::move(car), mass); // Agregar coche a la lista

    car->listener = this;
    startLineSensor = App->physics->CreateRectangleSensor(800, 650, 50, 10); // Sensor en la línea de meta
    startLineSensor->listener = this;

    return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
    UnloadTexture(MenuTexture);
    UnloadTexture(background);
	LOG("Unloading Intro scene");
	return true;
}
bool ModuleGame::MainMenu()
{
    if (isMenuActive)
    {
        DrawTexture(MenuTexture, 0, 0, WHITE);
        const char* menuOptions[] = { "Start", "Credits", "Exit" }; //opciones (maybe agregar elegir mapa?)
        const int totalOptions = 3; //num de opciones 
        for (int i = 0; i < totalOptions; ++i)
        {
            Color color = (i == selectedMenuOption) ? YELLOW : WHITE;
            int fontSize = (i == selectedMenuOption) ? 90 : 80;
            DrawText(menuOptions[i], 50, 700 + i * 100, fontSize, color);
        }
        // Manejo de entrada del menú
        if (IsKeyPressed(KEY_DOWN))
        {
            selectedMenuOption = (selectedMenuOption + 1) % totalOptions;
        }
        else if (IsKeyPressed(KEY_UP))
        {
            selectedMenuOption = (selectedMenuOption - 1 + totalOptions) % totalOptions;
        }
        else if (IsKeyPressed(KEY_ENTER)) //con enter se selecciona la opcion 
        {
            switch (selectedMenuOption)
            {
            case 0: // Start
                isMenuActive = false;
                break;
            case 1: // Credits
                showCredits = true;
                isMenuActive = false;
                break;
            case 2: // Exit
                exit(1);
            }
        }
    }
    if (showCredits)
    {
        DrawTexture(creditsTexture, 0, 0, WHITE);
        DrawText("Press BACKSPACE to return", 280, 950, 30, WHITE);
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            isMenuActive = true;
            showCredits = false;
        }
    }
}
update_status ModuleGame::Update()
{
    if (showCredits || isMenuActive)
    {
        MainMenu();
        return UPDATE_CONTINUE;
    }
    if (hasPassedStartLine)
    {
        b2Vec2 carPos = car->body->GetPosition();
        b2Vec2 sensorPos = startLineSensor->body->GetPosition();
        float distance = b2Distance(carPos, sensorPos);

        if (distance > 5.0f) // Umbral para estar lejos de la línea de meta
        {
            hasPassedStartLine = false;
        }
    }
    // Dibujar fondo y texto básico
    DrawTexture(background, 0, 0, WHITE);
    DrawText(TextFormat("Vueltas: %d/3", lapCount), 20, 20, 30, WHITE);


    // Turbo: actualizar estado y recargar si es necesario
    if (IsKeyDown(KEY_LEFT_SHIFT) && turboUsedTime < turboDuration) {
        // Turbo activo
        turboActive = true;
        turboUsedTime += GetFrameTime();
        if (turboUsedTime >= turboDuration) {
            turboUsedTime = turboDuration; // Limitar al máximo
            turboActive = false;           // Desactivar turbo si se gasta
        }
    }
    else {
        // Turbo no activo: recargar
        turboActive = false;
        turboRechargeTimer += GetFrameTime();
        if (turboRechargeTimer >= turboRechargeDuration) {
            turboRechargeTimer = turboRechargeDuration; // Limitar al máximo
        }
        // Reducir tiempo usado proporcionalmente
        if (turboUsedTime > 0.0f) {
            turboUsedTime -= GetFrameTime() * (turboDuration / turboRechargeDuration);
            if (turboUsedTime < 0.0f) {
                turboUsedTime = 0.0f;
            }
        }
    }

    // Dibujar coches y aplicar turbo
    for (Car& c : m_tdTire) {
        if (turboActive) {
            c.ApplyTurbo();
        }
        c.Update(m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]);
        c.Draw();
    }

    // **Indicador del Turbo**
    float usedPercentage = turboUsedTime / turboDuration;


    // Dibujar la barra
    DrawRectangle(50, 800, 20, 150, LIGHTGRAY); // Fondo de la barra
    DrawRectangle(50, 800 + 150 * usedPercentage, 20, 150 * (1.0f - usedPercentage), BLUE);
    // Indicador textual
    if (turboActive) {
        DrawText("Turbo Activo!", 20, 750, 20, GREEN);
    }
    else if (turboRechargeTimer <= turboRechargeDuration) {
        DrawText("Recargando Turbo!", 20, 750, 20, RED);
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
    float turboForce = 500.0f; // Fuerza adicional del turbo

    b2Vec2 forwardDirection = m_body->body->GetWorldVector(b2Vec2(0.0f, -2.0f));

    // Impulso en la dirección "delante"
    b2Vec2 turboImpulse = turboForce * forwardDirection;

    // Aplicar el impulso al centro del coche
    m_body->body->ApplyLinearImpulse(turboImpulse, m_body->body->GetWorldCenter(), true);
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
    // Obtén la posición y el ángulo del cuerpo físico
    b2Vec2 pos = m_body->body->GetPosition();        // Posición en el mundo físico
    float angle = m_body->body->GetAngle() * RAD2DEG; // Ángulo en grados

    // Dimensiones del coche (más pequeñas, en metros)
    float width = 1.0f;  // Ancho del coche en metros
    float height = 2.0f; // Alto del coche en metros

    // Convierte las dimensiones a píxeles
    float widthPixels = METERS_TO_PIXELS(width);
    float heightPixels = METERS_TO_PIXELS(height);

    // Convierte la posición del coche de metros a píxeles
    float posX = METERS_TO_PIXELS(pos.x);
    float posY = METERS_TO_PIXELS(pos.y);

    // Centro de rotación
    Vector2 origin = { widthPixels / 2.0f, heightPixels / 2.0f };

    // Dibuja el coche con rotación y tamaño reducido
    DrawRectanglePro(
        { posX, posY, widthPixels, heightPixels }, // Rectángulo con posición en píxeles
        origin,                                   // Punto de origen para la rotación
        angle,                                    // Ángulo de rotación
        RED                                       // Color del coche
    );
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
    float turningTorque = 0.5f; // Torque reducido para giros más controlados

    if (IsKeyDown(KEY_A)) {
        m_body->body->ApplyTorque(-turningTorque, true); // Gira a la izquierda
    }

    if (IsKeyDown(KEY_D)) {
        m_body->body->ApplyTorque(turningTorque, true); // Gira a la derecha
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

void Car::Update(float staticFriction, float dynamicFriction)
{
    // Parámetros ajustados del coche
    float maxSpeed = 3.0f;         // Velocidad máxima reducida
    float forwardForce = 50.0f;    // Fuerza aplicada para acelerar (muy reducida)
    float brakingForce = 80.0f;    // Fuerza aplicada para frenar
    float angularDamping = 0.02f;  // Amortiguación rotacional
    dynamicFriction = 1.0f;       // Fricción dinámica alta para detener rápidamente

    // Obtener la dirección hacia adelante del coche
    b2Vec2 forwardNormal = m_body->body->GetWorldVector(b2Vec2(0, 1));

    // **1. Control del jugador: Acelerar y frenar**
    b2Vec2 force(0, 0);
    if (IsKeyDown(KEY_W)) {
        force -= b2Vec2(forwardNormal.x * forwardForce, forwardNormal.y * forwardForce); // Aplicar fuerza hacia adelante
    }
    if (IsKeyDown(KEY_S)) {
        force += b2Vec2(forwardNormal.x * brakingForce, forwardNormal.y * brakingForce); // Aplicar fuerza hacia atrás (frenado)
    }


    float turningTorque = 1.3f; // Torque reducido para giros más controlados

    if (IsKeyDown(KEY_A)) {
        m_body->body->ApplyTorque(-turningTorque, true); // Gira a la izquierda
    }

    if (IsKeyDown(KEY_D)) {
        m_body->body->ApplyTorque(turningTorque, true); // Gira a la derecha
    }

    // **3. Fricción dinámica (frenado natural)**
    b2Vec2 velocity = m_body->body->GetLinearVelocity(); // Obtener la velocidad actual
    b2Vec2 friction = b2Vec2(
        -std::copysign(std::min(dynamicFriction, std::abs(velocity.x)), velocity.x), // Fricción en X
        -std::copysign(std::min(dynamicFriction, std::abs(velocity.y)), velocity.y)  // Fricción en Y
    );

    // Aplicar fuerzas al cuerpo (fuerza del usuario + fricción)
    m_body->body->ApplyForce(force + friction, m_body->body->GetWorldCenter(), true);

    // **4. Limitar velocidad máxima**
    if (velocity.Length() > maxSpeed) {
        velocity.Normalize();
        velocity *= maxSpeed;
        m_body->body->SetLinearVelocity(velocity);
    }

    // **5. Amortiguación angular**
    float angularImpulse = angularDamping * m_body->body->GetInertia() * -m_body->body->GetAngularVelocity();
    m_body->body->ApplyAngularImpulse(angularImpulse, true);
}
void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
    if (bodyA == startLineSensor && bodyB == car|| bodyA == car && bodyB == startLineSensor)
    {
        if (!hasPassedStartLine)
        {
            hasPassedStartLine = true; // Evitar dobles conteos
            lapCount++;
            LOG("Lap completed! Current lap count: %d", lapCount);
        }
    }
    // Manejo adicional para otros sensores
}

