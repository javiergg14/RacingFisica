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
    menuTexture = LoadTexture("Assets/Menu.png");
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
    selection = LoadSound("Assets/Sounds/SelectionMade.wav");
    switchOption = LoadSound("Assets/Sounds/SwitchOption.wav");
    countdownSound = LoadSound("Assets/Sounds/Countdown.wav");
    gasolina = LoadSound("Assets/Sounds/Gasolina.wav");
    mapSelection= LoadSound("Assets/Sounds/MapSelection.wav");
    turbo1 = LoadSound("Assets/Sounds/Turbo.wav");
    turbo2 = LoadSound("Assets/Sounds/Turbo.wav");
    motor = LoadSound("Assets/Sounds/Motor.wav");
    audiowin = LoadSound("Assets/Sounds/Audiowin.wav");
    raceSong = LoadSound("Assets/Sounds/Racesong.wav");
    LOG("Loading Intro assets");


    // Crear el coche en el centro de la pantalla
    mass = 100.0f; // Masa del coche

    car1 = new Car(App->physics->CreateRectangle(0, 0, 15, 25, 0, b2_dynamicBody), mass, 1, this);
    car2 = new Car(App->physics->CreateRectangle(0, 0, 15, 25, 0, b2_dynamicBody), mass, 2, this);

    car1->GetBody()->listener = this;
    car2->GetBody()->listener = this;

    m_tdTire.push_back(*car1); // Agregar coche 1
    m_tdTire.push_back(*car2); // Agregar coche 2

    // Configuración de checkpoints
    for (PhysBody* checkpoint : checkpoints)
    {
        checkpoint->listener = this;
    }

    return true;
}

// Load assets
bool ModuleGame::CleanUp()
{
    UnloadSound(turbo1);
    UnloadSound(turbo2);
    UnloadSound(mapSelection);
    UnloadSound(gasolina);
    UnloadSound(selection);
    UnloadSound(countdownSound);
    UnloadSound(motor);
    UnloadSound(audiowin);
    UnloadSound(raceSong);
    UnloadTexture(menuTexture);
    UnloadTexture(mapTextures[0]);
    UnloadTexture(mapTextures[1]);
    UnloadTexture(mapTextures[2]);
    UnloadTexture(car1Texture);
    UnloadTexture(car2Texture);
    UnloadTexture(creditsTexture);
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
        if (!IsSoundPlaying(gasolina))
        {
            PlaySound(gasolina);
        }
        // Dibuja el menú principal
        DrawTexture(menuTexture, 0, 0, WHITE);
        const char* menuOptions[] = { "Start", "Credits", "Controls", "Exit" };
        const int totalOptions = 4;
        for (int i = 0; i < totalOptions; ++i)
        {
            Color color = (i == selectedMenuOption) ? YELLOW : WHITE;
            int fontSize = (i == selectedMenuOption) ? 90 : 80;
            DrawText(menuOptions[i], 50, 600 + i * 100, fontSize, color);
        }

        // Manejo de entrada del menú
        if (IsKeyPressed(KEY_DOWN))
        {
            PlaySound(switchOption);
            selectedMenuOption = (selectedMenuOption + 1) % totalOptions;
        }
        else if (IsKeyPressed(KEY_UP))
        {
            PlaySound(switchOption);
            selectedMenuOption = (selectedMenuOption - 1 + totalOptions) % totalOptions;
        }
        else if (IsKeyPressed(KEY_ENTER) && !isEnterPressed)
        {
          
            PlaySound(selection);
            isEnterPressed = true;
            switch (selectedMenuOption)
            {
            case 0: // Start
                isMapSelectorActive = true;
                isMenuActive = false;
                    StopSound(gasolina);
                break;
            case 1: // Credits
                showCredits = true;
                isMenuActive = false;
                break;
            case 2: // Controls
                showControls = true;
                isMenuActive = false;
                break;
            case 3: // Exit
                exit(1);
            }
        }
    }
    //Show Credits Screen
    else if (showCredits)
    {
        DrawTexture(creditsTexture, 0, 0, WHITE);
        DrawText("Press BACKSPACE to return", 450, 950, 30, WHITE);
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            isMenuActive = true;
            showCredits = false;
        }
    }
    //Show Controls Screen
    else if (showControls)
    {
        DrawTexture(controlsTexture, 0, 0, WHITE);
        DrawText("Press BACKSPACE to return", 450, 950, 30, WHITE);
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            isMenuActive = true;
            showControls = false;
        }
    }
    //Map Selector Screen
    else if (isMapSelectorActive)
    {
        if (!IsSoundPlaying(mapSelection))
        {
            PlaySound(mapSelection);
        }
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            StopSound(mapSelection);
            isMenuActive = true;
            isMapSelectorActive = false;
        }
        DrawTexture(mapSelectorBgTexture, 0, 0, WHITE);

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
                const char* mapText = "Brewery Hills "; // Valor predeterminado
                if (i == 1) mapText = "Foamy Track";
                else if (i == 2) mapText = "Barrel Road";

                // Ajustar la posición del texto sobre el mapa seleccionado
                DrawText(mapText, 365, 775, 100, WHITE);
            }
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            PlaySound(switchOption);
            selectedMapIndex = (selectedMapIndex + 1) % 3;
        }
        else if (IsKeyPressed(KEY_LEFT)) {
            selectedMapIndex = (selectedMapIndex - 1 + 3) % 3;
            PlaySound(switchOption);
        }
        else if (IsKeyPressed(KEY_ENTER)) {
            StopSound(mapSelection);
            PlaySound(selection);
            isMapSelectorActive = false;

            car1LapCount = 1;
            car2LapCount = 1;
            car1TurboUsedTime = 0.0f;
            car2TurboUsedTime = 0.0f;
            car1CurrentCheckpointIndex = 0;
            car2CurrentCheckpointIndex = 0;
            totalTime = 0.0f;
            m_creationTimer.Start();

            CreateColliders();
            CreateCheckpoints();
            PlaySound(raceSong);
            PlaySound(motor);

            car1ActiveCheckpoints.resize(checkpoints.size(), true);
            car2ActiveCheckpoints.resize(checkpoints.size(), true);

            for (Car& c : m_tdTire) {
                SetInitPosCar(c);
            }
            // Inicializar contador
            countdownActive = true;
            countdownTimer = 0.0f;
            countdownValue = 3;
            playersCanMove = false;
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
    if (!bodyA || !bodyB || checkpoints.empty())
    {
        return;
    }

    for (size_t i = 0; i < checkpoints.size(); ++i)
    {
        if (bodyA == checkpoints[i] || bodyB == checkpoints[i])
        {
            // Coche 1
            if ((bodyA == car1->GetBody() || bodyB == car1->GetBody()) &&
                i < car1ActiveCheckpoints.size() && car1ActiveCheckpoints[i])
            {
                // Verificar si el coche ha pasado por todos los checkpoints anteriores
                if (i == car1CurrentCheckpointIndex) {
                    printf("Coche 1 ha tocado checkpoint %d. Checkpoint actual: %d\n", i, car1CurrentCheckpointIndex);
                    car1CurrentCheckpointIndex++;
                    CountLapsAndManageCheckpoints(car1LapCount, car1CurrentCheckpointIndex, car1ActiveCheckpoints, 1);
                }
            }
            // Coche 2
            else if ((bodyA == car2->GetBody() || bodyB == car2->GetBody()) &&
                i < car2ActiveCheckpoints.size() && car2ActiveCheckpoints[i])
            {
                // Verificar si el coche ha pasado por todos los checkpoints anteriores
                if (i == car2CurrentCheckpointIndex) {
                    printf("Coche 2 ha tocado checkpoint %d. Checkpoint actual: %d\n", i, car2CurrentCheckpointIndex);
                    car2CurrentCheckpointIndex++;
                    CountLapsAndManageCheckpoints(car2LapCount, car2CurrentCheckpointIndex, car2ActiveCheckpoints, 2);
                }
            }
            break;
        }
    }
}

void ModuleGame::CountLapsAndManageCheckpoints(int& lapCount, int& currentCheckpointIndex, std::vector<bool>& activeCheckpoints, int carNumber)
{
    if (currentCheckpointIndex >= checkpoints.size()) {
        currentCheckpointIndex = 0;
        lapCount++;
        printf("¡Coche %d ha completado una vuelta! Total de vueltas: %d\n", carNumber, lapCount);

        // Reactivar todos los checkpoints para la siguiente vuelta
        std::fill(activeCheckpoints.begin(), activeCheckpoints.end(), true);

        if (lapCount == 4)
        {
            gameFinished = true;
            StopSound(raceSong);
            StopSound(motor);
            PlaySound(audiowin);
            winner = carNumber;
            totalTime = m_creationTimer.ReadSec();
            printf("¡Coche %d ha terminado la carrera! Tiempo total: %.2f segundos\n", carNumber, totalTime);
        }
    }
    else {
        if (currentCheckpointIndex > 0) {
            printf("Desactivando checkpoint %d para coche %d\n", currentCheckpointIndex - 1, carNumber);
            activeCheckpoints[currentCheckpointIndex - 1] = false;
        }
    }
}
update_status ModuleGame::Update() {
    if (showCredits || isMenuActive || isMapSelectorActive || showControls) {
        MainMenu();
        return UPDATE_CONTINUE;
    }
    DrawTexture(mapTextures[selectedMapIndex], 0, 0, WHITE);

    for (Car& c : m_tdTire) {
        c.Draw(c.GetPlayer() == 1 ? car1Texture : car2Texture);
    }

    if (!gameFinished && playersCanMove) {
        for (Car& c : m_tdTire) {
            if ((c.GetPlayer() == 1 && car1TurboActive) || (c.GetPlayer() == 2 && car2TurboActive)) c.ApplyTurbo();
            c.Update();
            c.Draw(c.GetPlayer() == 1 ? car1Texture : car2Texture);
            c.DrawTurboTrail();
        }
    }

    if (gameFinished) {
        DrawText(TextFormat("PLAYER %d WINS!", winner), 290, 300, 100, GREEN);
        DrawText(TextFormat("Time: %.2f seconds", totalTime), 430, 400, 50, WHITE);
        DrawText("Press ENTER to continue", 460, 950, 30, WHITE);
        if (IsKeyPressed(KEY_ENTER)) {
            StopSound(audiowin);
            PlaySound(selection);
            RemoveMapColliders();
            gameFinished = false;
            for (Car& c : m_tdTire) {
                c.GetBody()->body->SetLinearVelocity({ 0, 0 });
                c.GetBody()->body->SetAngularVelocity({ 0 });
            }
            isMenuActive = true;
        }
        return UPDATE_CONTINUE;
    }
    else {
        DrawText(TextFormat("Lap: %d", car1LapCount), 20, 20, 30, WHITE);
        DrawText(TextFormat("Lap: %d", car2LapCount), 1250, 20, 30, WHITE);
    }

    if (IsKeyPressed(KEY_LEFT_SHIFT) && car1TurboUsedTime < turboDuration) car1TurboActive = true;
    if (car1TurboActive) {
        if (!IsSoundPlaying(turbo1)) PlaySound(turbo1);
        car1TurboUsedTime += GetFrameTime();
        if (car1TurboUsedTime >= turboDuration || IsKeyReleased(KEY_LEFT_SHIFT)) car1TurboActive = false;
        if (car1TurboUsedTime >= turboDuration) car1TurboUsedTime = turboDuration;
    }
    else {
        car1TurboRechargeTimer = fmin(car1TurboRechargeTimer + GetFrameTime(), turboRechargeDuration);
        if (car1TurboUsedTime > 0.0f) car1TurboUsedTime = fmax(car1TurboUsedTime - GetFrameTime() * (turboDuration / turboRechargeDuration), 0.0f);
        StopSound(turbo1);
    }

    if (IsKeyPressed(KEY_SPACE) && car2TurboUsedTime < turboDuration) car2TurboActive = true;
    if (car2TurboActive) {
        if(!IsSoundPlaying(turbo2)) PlaySound(turbo2);

        car2TurboUsedTime += GetFrameTime();
        if (car2TurboUsedTime >= turboDuration || IsKeyReleased(KEY_SPACE)) car2TurboActive = false;
        if (car2TurboUsedTime >= turboDuration) car2TurboUsedTime = turboDuration;
    }
    else {
        car2TurboRechargeTimer = fmin(car2TurboRechargeTimer + GetFrameTime(), turboRechargeDuration);
        if (car2TurboUsedTime > 0.0f) car2TurboUsedTime = fmax(car2TurboUsedTime - GetFrameTime() * (turboDuration / turboRechargeDuration), 0.0f);
        StopSound(turbo2);
    }

    float car1UsedPercentage = car1TurboUsedTime / turboDuration;
    DrawRectangle(50, 800, 20, 150, { 200, 200, 200, 100 });
    DrawRectangle(50, 800 + 150 * car1UsedPercentage, 20, 150 * (1.0f - car1UsedPercentage), BLUE);

    float car2UsedPercentage = car2TurboUsedTime / turboDuration;
    DrawRectangle(1300, 800, 20, 150, { 200, 200, 200, 100 });
    DrawRectangle(1300, 800 + 150 * car2UsedPercentage, 20, 150 * (1.0f - car2UsedPercentage), RED);

    if (countdownActive) {
        countdownTimer += GetFrameTime();
        if (!countdownSoundPlayed) {
            PlaySound(countdownSound);
            countdownSoundPlayed = true;
        }

        if (countdownTimer >= 1.0f) {
            countdownTimer = 0.0f;
            countdownValue--;
            if (countdownValue < 0) {
                countdownActive = false;
                playersCanMove = true;
            }
        }

        if (countdownValue > 0) DrawText(TextFormat("%d", countdownValue), 660, 400, 120, WHITE);
        else if (countdownValue == 0) DrawText("GO!", 610, 400, 120, GREEN);

        return UPDATE_CONTINUE;
    }

    return UPDATE_CONTINUE;
}
Collider::Collider(PhysBody* i_body)
    : m_body(i_body)
{
}
Collider::~Collider()
{ }
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
    float turboForce = 500000.0f;
    float turboMaxSpeed = 3000.0f; 

    // Obtener el vector de dirección del coche
    b2Vec2 forwardDirection = m_body->body->GetWorldVector(b2Vec2(0.0f, -1.0f));
    forwardDirection.Normalize();

    // Calcular el impulso basado en la fuerza del turbo
    if (m_body->body->GetLinearVelocity().Length() < turboMaxSpeed) {
        b2Vec2 turboImpulse = turboForce * forwardDirection;
        m_body->body->ApplyLinearImpulse(turboImpulse, m_body->body->GetWorldCenter(), true);
    }

    b2Vec2 velocity = m_body->body->GetLinearVelocity(); //velocidad del coche

    // Permitir una velocidad máxima mayor mientras el turbo está activo
    if (velocity.Length() > turboMaxSpeed) {
        velocity.Normalize();
        velocity *= turboMaxSpeed;
        m_body->body->SetLinearVelocity(velocity);
    }
}
Circle::~Circle()
{ }
Car::~Car()
{ }
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
    b2Vec2 pos = m_body->body->GetPosition();      
    float angle = m_body->body->GetAngle() * RAD2DEG; 

    float width = 23.0f; 
    float height = 43.0f; 

    float posX = METERS_TO_PIXELS(pos.x);
    float posY = METERS_TO_PIXELS(pos.y);

    // Centro de rotación
    Vector2 origin = { width / 2.0f, height / 2.0f };

    DrawTexturePro(texture, Rectangle{ 0, 0, (float)texture.width, (float)texture.height },
        Rectangle{ (float)posX, (float)posY, (float)texture.width, (float)texture.height },
        origin, angle, WHITE);
}
void Car::DrawTurboTrail()
{
    if ((player == 1 && m_moduleGame->car1TurboActive) || (player == 2 && m_moduleGame->car2TurboActive)) {

        // Actual Position of the Car
        b2Vec2 carPos = m_body->body->GetPosition();
        float angle = m_body->body->GetAngle();

        // Offset
        b2Vec2 trailOffset = m_body->body->GetWorldVector(b2Vec2(0.0f, 1.3f)); // Offset detrás del coche
        b2Vec2 trailPos = carPos + trailOffset;

        //position to pixels
        float posX = METERS_TO_PIXELS(trailPos.x);
        float posY = METERS_TO_PIXELS(trailPos.y);

        //iterate
        for (int i = 0; i < 8; ++i) {
            //Flame body & position
            float flameWidth = 10.0f - (i * 1.8f);    
            float flameHeight = 15.0f + (i * 6.0f);   
            float offsetX = (rand() % 5 - 2) / 10.0f;  
            float offsetY = -i * 5.0f / 10.0f;  

            // Dynamic Color
            Color flameColor = { 255, 150 - (i * 20), 0, 220 - (i * 25) };

            DrawRectanglePro( { posX + offsetX, posY + offsetY, flameWidth, flameHeight },{ flameWidth / 2.0f, flameHeight }, angle * RAD2DEG, flameColor );
        }
        // Dynamic Sparks
        for (int i = 0; i < 15; ++i) {
            float sparkX = posX + (rand() % 20 - 10);
            float sparkY = posY + (rand() % 10 - 5);
            float sparkSize = 1.0f + (rand() % 2);

            Color sparkColor = { 255, 200 + rand() % 55, 50 + rand() % 50, 180 + rand() % 75 };
            DrawCircle(sparkX, sparkY, sparkSize, sparkColor);
        }
        // Bright
        for (int i = 0; i < 2; ++i) { 
            float glowWidth = 20.f + (i * 5); 
            float glowHeight = 25.0f + (i * 10);

            Color glowColor = { 255, 200 + (i * 20), 0, 50 - (i * 15) };
            DrawRectanglePro({ posX, posY, glowWidth, glowHeight },{ glowWidth / 2.0f, glowHeight / 2.0f }, angle * RAD2DEG, glowColor);

        }
        // Distortion
        for (int i = 0; i < 6; ++i) {
            float distortionX = posX + (rand() % 8 - 4);
            float distortionY = posY + (i * 4);

            Color distortionColor = { 255, 255, 255, 50 + (rand() % 50) };
            DrawRectanglePro({ distortionX, distortionY }, { 3, 3 }, angle * RAD2DEG, distortionColor);
        }
    }
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
void Car::Update()
{
    // Parámetros ajustados del coche
    forwardForce =60.0f;  // Fuerza aplicada para acelerar
    brakingForce = 20.0f;  // Fuerza aplicada para frenar
    angularDamping = 0.1f; // Amortiguación rotacional
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
    if (selectedMapIndex == 0) {
        checkpoints.push_back(App->physics->CreateRectangleSensor(610, 115, 5, 200));
        checkpoints.push_back(App->physics->CreateRectangleSensor(125, 400, 203, 5));
        checkpoints.push_back(App->physics->CreateRectangleSensor(548, 652, 200, 5));
        checkpoints.push_back(App->physics->CreateRectangleSensor(832, 652, 205, 5)); 
        checkpoints.push_back(App->physics->CreateRectangleSensor(1250, 400, 203, 5));
        checkpoints.push_back(App->physics->CreateRectangleSensor(690, 115, 5, 200));
    }
    if (selectedMapIndex == 1) {
        checkpoints.push_back(App->physics->CreateRectangleSensor(610, 115, 5, 200)); // Meta Primero
        checkpoints.push_back(App->physics->CreateRectangleSensor(125, 400, 203, 5)); // Segundo
        checkpoints.push_back(App->physics->CreateRectangleSensor(610, 915, 5, 200)); // Tercero
        checkpoints.push_back(App->physics->CreateRectangleSensor(1235, 755, 200, 5));// Cuarto
        checkpoints.push_back(App->physics->CreateRectangleSensor(610, 537, 5, 200)); // Quinto
        checkpoints.push_back(App->physics->CreateRectangleSensor(350, 430, 200, 5)); //Sexta
        checkpoints.push_back(App->physics->CreateRectangleSensor(610, 330, 5, 200)); //Septima
        checkpoints.push_back(App->physics->CreateRectangleSensor(1250, 220, 200, 5));// Octava
        checkpoints.push_back(App->physics->CreateRectangleSensor(690, 115, 5, 200)); // Meta Ultima
    }
    if (selectedMapIndex == 2) {
        checkpoints.push_back(App->physics->CreateRectangleSensor(370, 115, 5, 200)); // Meta Primera
        checkpoints.push_back(App->physics->CreateRectangleSensor(580, 350, 200, 5)); // Segunda
        checkpoints.push_back(App->physics->CreateRectangleSensor(790, 650, 200, 5)); // Tercera
        checkpoints.push_back(App->physics->CreateRectangleSensor(1020, 915, 5, 200)); // Cuarto
        checkpoints.push_back(App->physics->CreateRectangleSensor(1020, 915, 5, 200)); // Quinto
        checkpoints.push_back(App->physics->CreateRectangleSensor(1250, 525, 200, 5)); // Sexto
        checkpoints.push_back(App->physics->CreateRectangleSensor(1020, 115, 5, 200)); // Septimo
        checkpoints.push_back(App->physics->CreateRectangleSensor(790, 350, 200, 5)); // Octavo
        checkpoints.push_back(App->physics->CreateRectangleSensor(580, 650, 200, 5)); // Novena
        checkpoints.push_back(App->physics->CreateRectangleSensor(330, 915, 5, 200)); // Decima
        checkpoints.push_back(App->physics->CreateRectangleSensor(125, 525, 203, 5)); // Onceava 
        checkpoints.push_back(App->physics->CreateRectangleSensor(290, 115, 5, 200)); // Meta Ultima
    }
}
void ModuleGame::CreateColliders()
{
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

    const int chain4Points[] = {
    228, 218,
    1145, 215,
    1143, 800,
    934, 800,
    930, 545,
    929, 526,
    904, 482,
    868, 440,
    826, 416,
    790, 402,
    586, 400,
    546, 421,
    506, 448,
    482, 481,
    460, 506,
    446, 540,
    444, 805,
    226, 810
    };

    const int chain3Points[] = {
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

    const int chain5Points[] = {
    230, 217,
    1142, 214,
    1140, 220,
    392, 227,
    337, 241,
    295, 274,
    275, 302,
    260, 335,
    251, 364,
    248, 395,
    244, 432,
    244, 468,
    254, 510,
    262, 547,
    290, 574,
    322, 602,
    350, 622,
    388, 631,
    1124, 638,
    1128, 810,
    225, 812
    };
    const int chain2Points[] = {
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
    const int chain6Points[] = {
            227, 215,
            478, 216,
            481, 401,
            593, 513,
            487, 607,
            483, 807,
            230, 807
    };
    const int chain7Points[] = {
            896, 221,
            1146, 218,
            1147, 807,
            890, 807,
            883, 610,
            787, 512,
            893, 403
    };
    if (selectedMapIndex == 0)
    {
        PhysBody* collider1 = App->physics->CreateChain(0, 0, chain1Points, sizeof(chain1Points) / sizeof(int));  
        m_colliders.emplace_back(collider1);
        PhysBody* collider4 = App->physics->CreateChain(0, 0, chain4Points, sizeof(chain4Points) / sizeof(int));
        m_colliders.emplace_back(collider4);

    }
    else if (selectedMapIndex == 1)
    {
        PhysBody* collider2 = App->physics->CreateChain(0, 0, chain2Points, sizeof(chain2Points) / sizeof(int));
        m_colliders.emplace_back(collider2);
        PhysBody* collider5 = App->physics->CreateChain(0, 0, chain5Points, sizeof(chain5Points) / sizeof(int));
        m_colliders.emplace_back(collider5);
    }
    else if (selectedMapIndex == 2)
    {
        PhysBody* collider3 = App->physics->CreateChain(0, 0, chain3Points, sizeof(chain3Points) / sizeof(int));
        m_colliders.emplace_back(collider3);
        PhysBody* collider6 = App->physics->CreateChain(0, 0, chain6Points, sizeof(chain6Points) / sizeof(int));
        m_colliders.emplace_back(collider6);
        PhysBody* collider7 = App->physics->CreateChain(0, 0, chain7Points, sizeof(chain7Points) / sizeof(int));
        m_colliders.emplace_back(collider7);

    }
}
PhysBody* Car::GetBody()
{
    return m_body;
}
void ModuleGame::SetInitPosCar(Car c)
{
    if (c.GetPlayer() == 1)
    {
        if (selectedMapIndex == 0)
        {
            c.GetBody()->body->SetTransform({ PIXEL_TO_METERS(650), PIXEL_TO_METERS(80) }, 270.0f * DEG2RAD);
        }
        else if (selectedMapIndex == 1)
        {
            c.GetBody()->body->SetTransform({ PIXEL_TO_METERS(650), PIXEL_TO_METERS(80) }, 270.0f * DEG2RAD);
        }
        else if (selectedMapIndex == 2)
        {
            c.GetBody()->body->SetTransform({ PIXEL_TO_METERS(330), PIXEL_TO_METERS(80) }, 90.0f * DEG2RAD);
        }
    }
    if (c.GetPlayer() == 2)
    {
        if (selectedMapIndex == 0)
        {
            c.GetBody()->body->SetTransform({ PIXEL_TO_METERS(650), PIXEL_TO_METERS(150) }, 270.0f * DEG2RAD);
        }
        else if (selectedMapIndex == 1)
        {
            c.GetBody()->body->SetTransform({ PIXEL_TO_METERS(650), PIXEL_TO_METERS(150) }, 270.0f * DEG2RAD);
        }
        else if (selectedMapIndex == 2)
        {
            c.GetBody()->body->SetTransform({ PIXEL_TO_METERS(330), PIXEL_TO_METERS(150) }, 90.0f * DEG2RAD);
        }
    }
    

}
void ModuleGame::RemoveMapColliders()
{
    // Iterar sobre los colliders
    for (auto it = m_colliders.begin(); it != m_colliders.end();)
    {
        Collider& collider = *it; // Obtener el collider actual
        // Comprobar si el collider es un coche
        if (collider.GetBody() != car1->GetBody() && collider.GetBody() != car2->GetBody())
        {
            App->physics->world->DestroyBody(collider.GetBody()->body); 
            it = m_colliders.erase(it); // Eliminar del vector
        }
        else
        {
            ++it; //avanzar si no se eliminó
        }
    }
    for (auto& checkpoint : checkpoints)//eliminar checkpoints
    {
        App->physics->world->DestroyBody(checkpoint->body);
    }
    checkpoints.clear(); // Limpiar la lista de checkpoints
}