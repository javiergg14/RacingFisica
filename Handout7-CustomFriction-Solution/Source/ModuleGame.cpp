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
    mapTextures[0] = LoadTexture("Assets/map1.png");
    mapTextures[1] = LoadTexture("Assets/map2.png");
    mapTextures[2] = LoadTexture("Assets/map3.png");
    mapSelectorBgTexture = LoadTexture("Assets/map_selector.png");
    creditsTexture = LoadTexture("Assets/Credits.png");
    car1Texture = LoadTexture("Assets/carPlayer1.png");
    car2Texture = LoadTexture("Assets/carPlayer2.png");
    mapSelectorTextures[0] = LoadTexture("Assets/map1selector.png");
    mapSelectorTextures[1] = LoadTexture("Assets/map2selector.png");
    mapSelectorTextures[2] = LoadTexture("Assets/map3selector.png");

    LOG("Loading Intro assets");
    bool ret = true;

    // Crear el coche en el centro de la pantalla
    mass = 100.0f; // Masa del coche

    car1 = new Car(App->physics->CreateRectangle(500, 500, 15, 25, b2_dynamicBody), mass, 1);
    car2 = new Car(App->physics->CreateRectangle(500, 500, 15, 25, b2_dynamicBody), mass, 2);

    m_tdTire.push_back(*car1); // Agregar coche 1
    m_tdTire.push_back(*car2); // Agregar coche 2

    // Configuración de checkpoints
    lapCount = 0;
    currentCheckpointIndex = 0;
    CreateCheckpoints();
    for (PhysBody* checkpoint : checkpoints)
    {
        checkpoint->listener = this;
    }

    CreateColliders();

    return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
    UnloadTexture(MenuTexture);
    UnloadTexture(mapTextures[0]);
    UnloadTexture(mapTextures[1]);
    UnloadTexture(mapTextures[2]);
    UnloadTexture(car1Texture);
    UnloadTexture(car2Texture);
    UnloadTexture(mapSelectorBgTexture);
    UnloadTexture(mapSelectorTextures[0]);
    UnloadTexture(mapSelectorTextures[1]);
    UnloadTexture(mapSelectorTextures[2]);

	LOG("Unloading Intro scene");
	return true;
}
bool ModuleGame::MainMenu()
{
    if (isMenuActive)
    {
        DrawTexture(MenuTexture, 0, 0, WHITE);
        const char* menuOptions[] = { "Start", "Credits", "Exit" };
        const int totalOptions = 3;
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
        else if (IsKeyPressed(KEY_ENTER) && !isEnterPressed) // Verificar si ENTER no está bloqueado
        {
            isEnterPressed = true; // Bloquea la tecla ENTER
            switch (selectedMenuOption)
            {
            case 0: // Start
                isMapSelectorActive = true;
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
    if (isMapSelectorActive)
    {
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            isMenuActive = true;
            isMapSelectorActive = false;
        }

        DrawTexture(mapSelectorBgTexture, 0, 0, WHITE);

        Vector2 mapPositions[3] = {
            {100, 400},
            {400, 400},
            {700, 400}
        };

        for (int i = 0; i < 3; ++i) {
            float scale = (i == selectedMapIndex) ? 1.5f : 1.0f;
            Vector2 position = mapPositions[i];
            Rectangle source = { 0, 0, (float)mapSelectorTextures[i].width, (float)mapSelectorTextures[i].height };
            Rectangle dest = {
                position.x,
                position.y,
                mapSelectorTextures[i].width * scale,
                mapSelectorTextures[i].height * scale
            };
            Vector2 origin = { mapSelectorTextures[i].width / 2.0f, mapSelectorTextures[i].height / 2.0f };
            DrawTexturePro(mapSelectorTextures[i], source, dest, origin, 0.0f, WHITE);
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            selectedMapIndex = (selectedMapIndex + 1) % 3;
        }
        else if (IsKeyPressed(KEY_LEFT)) {
            selectedMapIndex = (selectedMapIndex - 1 + 3) % 3;
        }
        else if (IsKeyPressed(KEY_ENTER) && !isEnterPressed) {
            isMapSelectorActive = false;
            // Aquí inicias el juego con el mapa seleccionado
        }
    }

    // Liberar el estado de la tecla ENTER cuando sea soltada
    if (IsKeyReleased(KEY_ENTER)) {
        isEnterPressed = false;
    }
}

// OnCollision para manejar colisiones
void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
    if (bodyA == car || bodyB == car)
    {
        for (size_t i = 0; i < checkpoints.size(); ++i)
        {
            if (bodyA == checkpoints[i] || bodyB == checkpoints[i])
            {
                // Checkpoint esperado
                if (i == currentCheckpointIndex)
                {
                    LOG("Checkpoint %d alcanzado!", currentCheckpointIndex + 1);
                    currentCheckpointIndex++;

                    // Si alcanzamos el último checkpoint y volvemos al de inicio/fin
                    if (currentCheckpointIndex >= checkpoints.size())
                    {
                        // Validar si estamos en el checkpoint de inicio/fin
                        if (i == 0)
                        {
                            lapCount++;
                            LOG("Vuelta Completada! Total de Vueltas: %d", lapCount);

                            if (lapCount == 3)
                            {
                                gameFinished = true;
                                totalTime = m_creationTimer.ReadSec();
                            }

                        }
                        currentCheckpointIndex = 0; // Reiniciar al primer checkpoint
                    }
                }
                else
                {
                    LOG("Checkpoint %d ignorado.", i + 1);
                }

                break;
            }
        }
    }
}

update_status ModuleGame::Update()
{
    if (showCredits || isMenuActive || isMapSelectorActive)
    {
        MainMenu();
        return UPDATE_CONTINUE;
    }

    // Dibujar fondo y texto básico
    DrawTexture(mapTextures[selectedMapIndex], 0, 0, WHITE);
    DrawText(TextFormat("Laps: %d", lapCount), 20, 20, 30, WHITE);

    // Win text si el juego ha terminado
    if (gameFinished) {
        DrawText("YOU WIN", 250, 300, 50, GREEN);
        DrawText(TextFormat("Time: %.2f seconds", totalTime), 250, 400, 30, WHITE);
        return UPDATE_CONTINUE;
    }

    // Turbo: actualizar estado y recargar si es necesario
    if (turboActive) {
        turboUsedTime += GetFrameTime();
        if (turboUsedTime >= turboDuration) {
            turboActive = false;
            turboUsedTime = turboDuration;
        }

        // Desactivar turbo si se suelta la tecla SHIFT
        if (IsKeyReleased(KEY_LEFT_SHIFT)) {
            turboActive = false;
        }
    }
    else {
        // Recargar turbo
        turboRechargeTimer += GetFrameTime();
        if (turboRechargeTimer > turboRechargeDuration) {
            turboRechargeTimer = turboRechargeDuration;
        }
        // Reducir tiempo usado si no se está activando
        if (turboUsedTime > 0.0f) {
            turboUsedTime -= GetFrameTime() * (turboDuration / turboRechargeDuration);
            if (turboUsedTime < 0.0f) {
                turboUsedTime = 0.0f;
            }
        }
    }

    // Activar turbo si se presiona la tecla y hay energía
    if (IsKeyPressed(KEY_LEFT_SHIFT) && turboRechargeTimer >= turboDuration) {
        turboActive = true;
        turboRechargeTimer = 0.0f;
    }

    // Dibujar coches y aplicar turbo
    for (Car& c : m_tdTire)
    {
        if (turboActive) {
            c.ApplyTurbo();
        }
        c.Update(m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]);

        if (c.GetPlayer() == 1)
        {
            c.Draw(car1Texture);
        }
        else if (c.GetPlayer() == 2) {
            c.Draw(car2Texture);
        }
    }

    // **Indicador del Turbo**
    float usedPercentage = turboUsedTime / turboDuration;

    // Barra de recarga del turbo
    DrawRectangle(50, 800, 20, 150, LIGHTGRAY); // Fondo de la barra
    DrawRectangle(50, 800 + 150 * usedPercentage, 20, 150 * (1.0f - usedPercentage), BLUE); // Turbo restante

    // Indicador textual
    if (turboActive) {
        DrawText("Turbo Activo!", 20, 750, 20, GREEN);
    }
    else {
        DrawText("Recargando Turbo!", 20, 750, 20, RED);
    }

    for (Collider& collider : m_colliders)
    {
        collider.Draw();
    }

    return UPDATE_CONTINUE;
}

Collider::Collider(PhysBody* i_body)
    : m_body(i_body)
{
}

Collider::~Collider()
{
}

void Collider::Draw()
{
    if (m_body)
    {
        // Obtén los vértices del collider como un vector de b2Vec2
        const std::vector<b2Vec2>& vertices = m_body->GetVertices();

        // Recorre los vértices para dibujar las líneas
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            b2Vec2 start = vertices[i];
            b2Vec2 end = vertices[(i + 1) % vertices.size()]; // Conectar el último con el primero

            DrawLine(
                METERS_TO_PIXELS(start.x), METERS_TO_PIXELS(start.y),
                METERS_TO_PIXELS(end.x), METERS_TO_PIXELS(end.y),
                RED
            );
        }
    }
}


Circle::Circle(PhysBody* i_body, float i_mass)
	: m_body(i_body)
	, mass(i_mass)
{
	m_lifeTime.Start();
}

Car::Car(PhysBody* i_body, float i_mass, int i_player)
    : m_body(i_body), mass(i_mass), player(i_player)
{
    m_lifeTime.Start();
}
void Car::ApplyTurbo()
{
    float turboForce = 500.0f; // Fuerza adicional del turbo

    // Obtener el vector de dirección "adelante" del coche en el mundo
    b2Vec2 forwardDirection = m_body->body->GetWorldVector(b2Vec2(0.0f, -1.0f));

    // Normalizar el vector para evitar problemas con magnitud
    forwardDirection.Normalize();

    // Calcular el impulso basado en la fuerza del turbo
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

int Car::GetPlayer()
{
    return player;
}

void Circle::Draw()
{
	b2Vec2 pos = m_body->body->GetPosition();
	DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), (float)METERS_TO_PIXELS(std::log(mass)), Color{ 128, 0, 0, 128 });

}

void Car::Draw(Texture2D texture)
{
    // Obtén la posición y el ángulo del cuerpo físico
    b2Vec2 pos = m_body->body->GetPosition();        // Posición en el mundo físico
    float angle = m_body->body->GetAngle() * RAD2DEG; // Ángulo en grados

    // Dimensiones del coche (más pequeñas, en metros)
    float width = 15.0f;  // Ancho del coche en metros
    float height = 25.0f; // Alto del coche en metro

    // Convierte la posición del coche de metros a píxeles
    float posX = METERS_TO_PIXELS(pos.x);
    float posY = METERS_TO_PIXELS(pos.y);

    // Centro de rotación
    Vector2 origin = { width / 2.0f, height / 2.0f };

    DrawTexturePro(texture, Rectangle{ 0, 0, (float)texture.width, (float)texture.height },
        Rectangle{ (float)posX, (float)posY, (float)texture.width, (float)texture.height },
        origin, angle, WHITE);
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
    float maxSpeed = 2.0f;         // Velocidad máxima reducida
    float forwardForce = 1.0f;    // Fuerza aplicada para acelerar (muy reducida)
    float brakingForce = 0.5f;    // Fuerza aplicada para frenar
    float angularDamping = 0.1f;  // Amortiguación rotacional
    dynamicFriction = 0.15f;       // Fricción dinámica alta para detener rápidamente

    // Obtener la dirección hacia adelante del coche
    b2Vec2 forwardNormal = m_body->body->GetWorldVector(b2Vec2(0, 1));

    // **1. Control del jugador: Acelerar y frenar**
    b2Vec2 force(0, 0);
    if (player == 1) { // Controles WASD
        if (IsKeyDown(KEY_W)) {
            force -= b2Vec2(forwardNormal.x * forwardForce, forwardNormal.y * forwardForce);
        }
        if (IsKeyDown(KEY_S)) {
            force += b2Vec2(forwardNormal.x * brakingForce, forwardNormal.y * brakingForce);
        }
        if (IsKeyDown(KEY_A)) {
            m_body->body->ApplyTorque(-0.03f, true); // Gira a la izquierda
        }
        if (IsKeyDown(KEY_D)) {
            m_body->body->ApplyTorque(0.03f, true); // Gira a la derecha
        }
    }
    else if (player == 2) { // Controles de flechas
        if (IsKeyDown(KEY_UP)) {
            force -= b2Vec2(forwardNormal.x * forwardForce, forwardNormal.y * forwardForce);
        }
        if (IsKeyDown(KEY_DOWN)) {
            force += b2Vec2(forwardNormal.x * brakingForce, forwardNormal.y * brakingForce);
        }
        if (IsKeyDown(KEY_LEFT)) {
            m_body->body->ApplyTorque(-0.03f, true); // Gira a la izquierda
        }
        if (IsKeyDown(KEY_RIGHT)) {
            m_body->body->ApplyTorque(0.03f, true); // Gira a la derecha
        }
    }

    // **2. Fricción estática**
    b2Vec2 velocity = m_body->body->GetLinearVelocity(); // Obtener la velocidad actual
    if (velocity.LengthSquared() < 0.001f) { // Si el coche está prácticamente quieto
        float N = mass * 9.8f; // Fuerza normal aproximada
        float staticFrictionForce = N * staticFriction; // Fuerza de fricción estática
        b2Vec2 friction = b2Vec2(
            -std::copysign(std::min(staticFrictionForce, std::abs(force.x)), force.x),
            -std::copysign(std::min(staticFrictionForce, std::abs(force.y)), force.y)
        );
        force += friction; // Añadir la fricción estática a la fuerza total
    }

    // **3. Fricción dinámica (frenado natural)**
    b2Vec2 friction = b2Vec2(
        -std::copysign(std::min(dynamicFriction, std::abs(velocity.x)), velocity.x), // Fricción en X
        -std::copysign(std::min(dynamicFriction, std::abs(velocity.y)), velocity.y)  // Fricción en Y
    );

    // Aplicar fuerzas al cuerpo (fuerza del usuario + fricción dinámica)
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

    checkpoints.push_back(checkpoints[0]); // Checkpoint final es el mismo que el inicio
}

void ModuleGame::CreateColliders()
{
    // Coordenadas de los puntos para los colisionadores
    // Cada par (x, y) define un punto, y el último conecta al primero (loop)
    const int chain1Points[] = { 100, 100, 200, 100, 200, 200, 100, 200 }; // Un cuadrado
    const int chain2Points[] = { 300, 300, 400, 300, 400, 400, 300, 400 }; // Otro cuadrado

    // Crear colisionadores usando la función CreateChain
    PhysBody* collider1 = App->physics->CreateChain(0, 0, chain1Points, sizeof(chain1Points) / sizeof(int));
    PhysBody* collider2 = App->physics->CreateChain(0, 0, chain2Points, sizeof(chain2Points) / sizeof(int));

    // Añadir los colisionadores a la lista
    m_colliders.emplace_back(collider1);
    m_colliders.emplace_back(collider2);
}