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
    controlsTexture = LoadTexture("Assets/Controls.png");
    Selection = LoadSound("Assets/Sounds/SelectionMade.wav");
    SwitchOption = LoadSound("Assets/Sounds/SwitchOption.wav");

    LOG("Loading Intro assets");
    bool ret = true;

    // Crear el coche en el centro de la pantalla
    mass = 100.0f; // Masa del coche

    car1 = new Car(App->physics->CreateRectangle(500, 500, 15, 25, b2_dynamicBody), mass, 1,this);
    car2 = new Car(App->physics->CreateRectangle(500, 500, 15, 25, b2_dynamicBody), mass, 2,this);

    car1->GetBody()->listener = this;
    car2->GetBody()->listener = this;

    m_tdTire.push_back(*car1); // Agregar coche 1
    m_tdTire.push_back(*car2); // Agregar coche 2

    // Configuración de checkpoints
    lapCount = 0;
    currentCheckpointIndex = 0;
    for (PhysBody* checkpoint : checkpoints)
    {
        checkpoint->listener = this;
    }

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
        // Dibuja el menú principal
        DrawTexture(MenuTexture, 0, 0, WHITE);
        const char* menuOptions[] = { "Start", "Credits", "Controls", "Exit" }; // Añadir "Controls"
        const int totalOptions = 4; // Actualizar el total de opciones
        for (int i = 0; i < totalOptions; ++i)
        {
            Color color = (i == selectedMenuOption) ? YELLOW : WHITE;
            int fontSize = (i == selectedMenuOption) ? 90 : 80;
            DrawText(menuOptions[i], 50, 600 + i * 100, fontSize, color);
        }

        // Manejo de entrada del menú
        if (IsKeyPressed(KEY_DOWN))
        {
            PlaySound(SwitchOption);
            selectedMenuOption = (selectedMenuOption + 1) % totalOptions;
        }
        else if (IsKeyPressed(KEY_UP))
        {
            PlaySound(SwitchOption);
            selectedMenuOption = (selectedMenuOption - 1 + totalOptions) % totalOptions;
        }
        else if (IsKeyPressed(KEY_ENTER) && !isEnterPressed)
        {
            PlaySound(Selection);
            isEnterPressed = true;
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
            case 2: // Controls
                showControls = true; // Activar la pantalla de controles
                isMenuActive = false; // Desactivar el menú principal
                break;
            case 3: // Exit
                exit(1);
            }
        }
    }
    else if (showCredits)
    {
        DrawTexture(creditsTexture, 0, 0, WHITE);
        DrawText("Press BACKSPACE to return", 450, 950, 30, WHITE);
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            isMenuActive = true;
            showCredits = false; // Regresar al menú
        }
    }
    else if (showControls) // Mostrar los controles
    {
        DrawTexture(controlsTexture, 0, 0, WHITE); // Dibuja la textura de controles
        DrawText("Press BACKSPACE to return", 450, 950, 30, WHITE); // Instrucciones para regresar
        if (IsKeyPressed(KEY_BACKSPACE)) // Regresar al menú principal
        {
            isMenuActive = true;
            showControls = false; // Desactivar la pantalla de controles
        }
    }
    else if (isMapSelectorActive)
    {
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            isMenuActive = true;
            isMapSelectorActive = false;
        }
        DrawTexture(mapSelectorBgTexture, 0, 0, WHITE);
        Vector2 mapPositions[3] = {
            {200, 400},
            {650, 400},
            {1100, 400}
        };

        for (int i = 0; i < 3; ++i) {
            float scale = (i == selectedMapIndex) ? 1.25f : 1.0f;
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

            // Mostrar el texto solo si el mapa está seleccionado
            if (i == selectedMapIndex) {
                const char* mapText = "Map 1: Brewery Hills "; // Valor predeterminado
                if (i == 1) mapText = "Map 2: Foamy Track";
                else if (i == 2) mapText = "Map 3: Barrel Road";

                // Ajustar la posición del texto sobre el mapa seleccionado
                DrawText(mapText, 450, 700, 50, WHITE);
            }
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            PlaySound(SwitchOption);
            selectedMapIndex = (selectedMapIndex + 1) % 3;
        }
        else if (IsKeyPressed(KEY_LEFT)) {
            selectedMapIndex = (selectedMapIndex - 1 + 3) % 3;
            PlaySound(SwitchOption);
        }
        else if (IsKeyPressed(KEY_ENTER))
        {
            PlaySound(Selection);
            isMapSelectorActive = false;
            CreateColliders();
            CreateCheckpoints();
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
    // Verificar colisiones con car1 y car2
    if (bodyA == car1->GetBody() || bodyB == car1->GetBody())
    {
        LOG("Colisión detectada para el coche 1");
    }
    else if (bodyA == car2->GetBody() || bodyB == car2->GetBody())
    {
        LOG("Colisión detectada para el coche 2");
    }

    // Manejo de otros cuerpos físicos, como checkpoints
    for (size_t i = 0; i < checkpoints.size(); ++i)
    {
        if (bodyA == checkpoints[i] || bodyB == checkpoints[i])
        {
            printf("hola");
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
update_status ModuleGame::Update()
{
    if (showCredits || isMenuActive || isMapSelectorActive||showControls)
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
    // Turbo para car1
    if (IsKeyPressed(KEY_LEFT_SHIFT) && car1TurboUsedTime < turboDuration) {
        car1TurboActive = true;
    }
    // Manejo de tiempo de uso y desactivación
    if (car1TurboActive) {
        car1TurboUsedTime += GetFrameTime();
        if (car1TurboUsedTime >= turboDuration) {
            car1TurboActive = false;
            car1TurboUsedTime = turboDuration;
        }
        if (IsKeyReleased(KEY_LEFT_SHIFT)) {
            car1TurboActive = false;
        }
    }
    else {
        car1TurboRechargeTimer += GetFrameTime();
        if (car1TurboRechargeTimer > turboRechargeDuration) {
            car1TurboRechargeTimer = turboRechargeDuration;
        }
        if (car1TurboUsedTime > 0.0f) {
            car1TurboUsedTime -= GetFrameTime() * (turboDuration / turboRechargeDuration);
            if (car1TurboUsedTime < 0.0f) {
                car1TurboUsedTime = 0.0f;
            }
        }
    }
    // Turbo para car2
    if (IsKeyPressed(KEY_RIGHT_SHIFT) && car2TurboUsedTime < turboDuration) {
        car2TurboActive = true;
    }
    if (car2TurboActive) {
        car2TurboUsedTime += GetFrameTime();
        if (car2TurboUsedTime >= turboDuration) {
            car2TurboActive = false;
            car2TurboUsedTime = turboDuration;
        }
        if (IsKeyReleased(KEY_RIGHT_SHIFT)) {
            car2TurboActive = false;
        }
    }
    else {
        car2TurboRechargeTimer += GetFrameTime();
        if (car2TurboRechargeTimer > turboRechargeDuration) {
            car2TurboRechargeTimer = turboRechargeDuration;
        }
        if (car2TurboUsedTime > 0.0f) {
            car2TurboUsedTime -= GetFrameTime() * (turboDuration / turboRechargeDuration);
            if (car2TurboUsedTime < 0.0f) {
                car2TurboUsedTime = 0.0f;
            }
        }
    }
    // Dibujar coches y aplicar turbo
    for (Car& c : m_tdTire)
    {
        if ((c.GetPlayer() == 1 && car1TurboActive) || (c.GetPlayer() == 2 && car2TurboActive)) {
            c.ApplyTurbo();
        }
        c.Update(m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]);

        if (c.GetPlayer() == 1) {
            c.Draw(car1Texture);
        }
        else if (c.GetPlayer() == 2) {
            c.Draw(car2Texture);
        }
    }
    // Indicadores de turbo para car1
    float car1UsedPercentage = car1TurboUsedTime / turboDuration;
    DrawRectangle(50, 800, 20, 150, LIGHTGRAY);
    DrawRectangle(50, 800 + 150 * car1UsedPercentage, 20, 150 * (1.0f - car1UsedPercentage), BLUE);

    // Indicadores de turbo para car2
    float car2UsedPercentage = car2TurboUsedTime / turboDuration;
    DrawRectangle(100, 800, 20, 150, LIGHTGRAY);
    DrawRectangle(100, 800 + 150 * car2UsedPercentage, 20, 150 * (1.0f - car2UsedPercentage), RED);

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

Car::Car(PhysBody* i_body, float i_mass, int i_player, ModuleGame* moduleGame)
    : m_body(i_body), mass(i_mass), player(i_player), m_moduleGame(moduleGame)
{
    m_lifeTime.Start();
}
void Car::ApplyTurbo() {
    float turboForce = 500000.0f; // Fuerza adicional del turbo
    float turboMaxSpeed = 300.0f; // Velocidad máxima durante el turbo (más alta que la velocidad normal)

    // Obtener el vector de dirección "adelante" del coche en el mundo
    b2Vec2 forwardDirection = m_body->body->GetWorldVector(b2Vec2(0.0f, -1.0f));
    forwardDirection.Normalize();

    // Calcular el impulso basado en la fuerza del turbo
    if (m_body->body->GetLinearVelocity().Length() < turboMaxSpeed) {
        b2Vec2 turboImpulse = turboForce * forwardDirection;
        m_body->body->ApplyLinearImpulse(turboImpulse, m_body->body->GetWorldCenter(), true);
    }

    // Obtener la velocidad actual del coche
    b2Vec2 velocity = m_body->body->GetLinearVelocity();

    // Permitir temporalmente una velocidad máxima mayor mientras el turbo está activo
    if (velocity.Length() > turboMaxSpeed) {
        velocity.Normalize();
        velocity *= turboMaxSpeed;
        m_body->body->SetLinearVelocity(velocity);
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
    float width = 23.0f;  // Ancho del coche en metros
    float height = 43.0f; // Alto del coche en metro

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
    float forwardForce = 50.0f;  // Fuerza aplicada para acelerar
    float brakingForce = 25.0f;  // Fuerza aplicada para frenar
    float angularDamping = 0.1f; // Amortiguación rotacional
    dynamicFriction = 0.2f;    // Fricción dinámica ajustada

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
            m_body->body->ApplyTorque(-0.07f, true); // Gira a la izquierda
        }
        if (IsKeyDown(KEY_D)) {
            m_body->body->ApplyTorque(0.07f, true); // Gira a la derecha
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
            m_body->body->ApplyTorque(-0.07f , true); // Gira a la izquierda
        }
        if (IsKeyDown(KEY_RIGHT)) {
            m_body->body->ApplyTorque(0.07f, true); // Gira a la derecha
        }
    }

    // **2. Fricción estática**
    b2Vec2 velocity = m_body->body->GetLinearVelocity(); // Obtener la velocidad actual
    if (velocity.LengthSquared() < 0.001f) { // Si el coche está prácticamente quieto
        float N = mass * 9.8f; // Fuerza normal aproximada
        float staticFrictionForce = N * staticFriction; // Fuerza de fricción estática ajustada
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
    float maxAllowedSpeed = (m_moduleGame->car1TurboActive && player == 1) ||
        (m_moduleGame->car2TurboActive && player == 2) ? 5.5f : 2.0f;

    if (velocity.Length() > maxAllowedSpeed) {
        velocity.Normalize();
        velocity *= maxAllowedSpeed;
        m_body->body->SetLinearVelocity(velocity);
    }

    // **5. Amortiguación angular**
    float angularImpulse = angularDamping * m_body->body->GetInertia() * -m_body->body->GetAngularVelocity();
    m_body->body->ApplyAngularImpulse(angularImpulse, true);
}
void ModuleGame::CreateCheckpoints()
{
    // Checkpoint 1: Inicio/Fin
    if (selectedMapIndex == 0)
    {
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

        checkpoints.push_back(checkpoints[0]); // Checkpoint final es el mismo que el inicio
    }
    if (selectedMapIndex == 1) {

    }
    if (selectedMapIndex == 2)
    {

    }
  
}

void ModuleGame::CreateColliders()
{
    // Coordenadas de los puntos para los colisionadores
    // Cada par (x, y) define un punto, y el último conecta al primero (loop)i
    const int chain1Points[] = {
    487, 1010,
    515, 1006,
    535, 998,
    564, 983,
    580, 972,
    604, 946,
    623, 923,
    644, 872,
    649, 609,
    724, 607,
    729, 853,
    740, 895,
    753, 924,
    770, 938,
    795, 960,
    815, 981,
    846, 998,
    876, 1004,
    1204, 1004,
    1250, 989,
    1281, 963,
    1307, 936,
    1332, 903,
    1350, 867,
    1355, 815,
    1352, 166,
    1329, 107,
    1290, 56,
    1227, 23,
    1172, 12,
    175, 13,
    112, 40,
    76, 67,
    41, 107,
    29, 143,
    23, 863,
    36, 904,
    56, 946,
    84, 969,
    107, 990,
    150, 1004,
    201, 1012,
    298, 1010,
    481, 1006
    };
    const int chain2Points[] = {
    21, 168,
    23, 870,
    76, 970,
    163, 1011,
    535, 1016,
    636, 955,
    686, 871,
    686, 618,
    683, 876,
    738, 976,
    853, 1016,
    1208, 1015,
    1306, 961,
    1353, 876,
    1353, 160,
    1310, 63,
    1215, 15,
    823, 13,
    711, 83,
    683, 146,
    685, 391,
    683, 120,
    565, 11,
    153, 10,
    56, 71,
    20, 168
    };
    const int chain3Points[] = {
    24, 157,
    22, 878,
    67, 967,
    162, 1017,
    1190, 1014,
    1298, 952,
    1332, 870,
    1338, 577,
    1262, 467,
    1190, 431,
    460, 431,
    1187, 430,
    1285, 391,
    1350, 294,
    1345, 151,
    1300, 64,
    1217, 11,
    171, 8,
    67, 64,
    25, 157
    };


    if (selectedMapIndex == 0)
    {
        PhysBody* collider1 = App->physics->CreateChain(0, 0, chain1Points, sizeof(chain1Points) / sizeof(int));  
        m_colliders.emplace_back(collider1);
    }
    else if (selectedMapIndex == 1)
    {
        PhysBody* collider2 = App->physics->CreateChain(0, 0, chain2Points, sizeof(chain2Points) / sizeof(int));
        m_colliders.emplace_back(collider2);
    }
    else if (selectedMapIndex == 2)
    {
        PhysBody* collider3 = App->physics->CreateChain(0, 0, chain3Points, sizeof(chain3Points) / sizeof(int));
        m_colliders.emplace_back(collider3);
    }
}
PhysBody* Car::GetBody()
{
    return m_body;
}