#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#include <math.h>
#include <chrono>
#include <thread>



ModulePhysics::ModulePhysics(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	world = NULL;
	debug = false;
}

// Destructor
ModulePhysics::~ModulePhysics()
{
}

bool ModulePhysics::Start()
{
	LOG("Creating Physics 2D environment");
	
	world = new b2World(b2Vec2(GRAVITY_X, -GRAVITY_Y));
	world->SetContactListener(this);
	return true;
}

// 
update_status ModulePhysics::PreUpdate()
{
	// Inicia un temporizador para medir la duración de cada frame
	static auto lastFrameTime = std::chrono::high_resolution_clock::now();

	// Llama al Step del mundo de Box2D
	world->Step(1.0f / 60.0f, 6, 2);

	for (b2Contact* c = world->GetContactList(); c; c = c->GetNext())
	{
		if (c->GetFixtureA()->IsSensor() && c->IsTouching())
		{
			b2BodyUserData data1 = c->GetFixtureA()->GetBody()->GetUserData();
			b2BodyUserData data2 = c->GetFixtureB()->GetBody()->GetUserData();

			PhysBody* pb1 = (PhysBody*)data1.pointer;
			PhysBody* pb2 = (PhysBody*)data2.pointer;
			if (pb1 && pb2 && pb1->listener)
				pb1->listener->OnCollision(pb1, pb2);
		}
		else if (c->GetFixtureB()->IsSensor() && c->IsTouching())
		{
			b2BodyUserData data1 = c->GetFixtureB()->GetBody()->GetUserData();
			b2BodyUserData data2 = c->GetFixtureA()->GetBody()->GetUserData();

			PhysBody* pb1 = (PhysBody*)data1.pointer;
			PhysBody* pb2 = (PhysBody*)data2.pointer;
			if (pb1 && pb2 && pb1->listener)
				pb1->listener->OnCollision(pb1, pb2);
		}
	}

	// Calcula el tiempo transcurrido desde el último frame
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsedTime = currentTime - lastFrameTime;

	// Duración ideal de un frame (1/60 segundos)
	const float frameDuration = 1.0f / 60.0f;

	// Si el frame actual se procesó más rápido que el objetivo, duerme el tiempo restante
	if (elapsedTime.count() < frameDuration) {
		std::this_thread::sleep_for(std::chrono::duration<float>(frameDuration - elapsedTime.count()));
	}

	// Actualiza el tiempo del último frame
	lastFrameTime = std::chrono::high_resolution_clock::now();

	return UPDATE_CONTINUE;
}

update_status ModulePhysics::PostUpdate()
{
	if (IsKeyPressed(KEY_F1))
	{
		debug = !debug;
	}
	if (!debug)
	{
		return UPDATE_CONTINUE;
	}
	// Iterar sobre todos los cuerpos en el mundo
	for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
	{
		for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
		{
			// Comprobar si la fixture es un sensor
			if (f->IsSensor())
			{
				b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();
				b2Vec2 prev, v;

				int32 count = polygonShape->m_count;
				for (int32 i = 0; i < count; ++i)
				{
					v = b->GetWorldPoint(polygonShape->m_vertices[i]);
					if (i > 0)
						DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), BLUE);

					prev = v;
				}

				v = b->GetWorldPoint(polygonShape->m_vertices[0]);
				DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), BLUE);

				continue; // Evitar dibujar como otros tipos
			}

			// Dibujar seg?n el tipo de fixture
			switch (f->GetType())
			{
			case b2Shape::e_circle:
			{
				b2CircleShape* shape = (b2CircleShape*)f->GetShape();
				b2Vec2 pos = f->GetBody()->GetPosition();
				DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), (float)METERS_TO_PIXELS(shape->m_radius), Color{ 0, 0, 0, 128 });
			}
			break;
			case b2Shape::e_polygon:
			{
				b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();
				int32 count = polygonShape->m_count;
				b2Vec2 prev, v;

				for (int32 i = 0; i < count; ++i)
				{
					v = b->GetWorldPoint(polygonShape->m_vertices[i]);
					if (i > 0)
						DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), RED);

					prev = v;
				}

				v = b->GetWorldPoint(polygonShape->m_vertices[0]);
				DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), RED);
			}
			break;

			case b2Shape::e_chain:
			{
				b2ChainShape* shape = (b2ChainShape*)f->GetShape();
				b2Vec2 prev, v;

				for (int32 i = 0; i < shape->m_count; ++i)
				{
					v = b->GetWorldPoint(shape->m_vertices[i]);
					if (i > 0)
						DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), GREEN);

					prev = v;
				}

				v = b->GetWorldPoint(shape->m_vertices[0]);
				DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), GREEN);
			}
			break;

			case b2Shape::e_edge:
			{
				b2EdgeShape* shape = (b2EdgeShape*)f->GetShape();
				b2Vec2 v1 = b->GetWorldPoint(shape->m_vertex1);
				b2Vec2 v2 = b->GetWorldPoint(shape->m_vertex2);
				DrawLine(METERS_TO_PIXELS(v1.x), METERS_TO_PIXELS(v1.y), METERS_TO_PIXELS(v2.x), METERS_TO_PIXELS(v2.y), BLUE);
			}
			break;
			}
		}
	}
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModulePhysics::CleanUp()
{
	LOG("Destroying physics world");

	// Delete the whole physics world!
	delete world;

	return true;
}

PhysBody* ModulePhysics::CreateCircle(int x, int y, int radius)
{
	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	b2Body* b = world->CreateBody(&body);
	b2CircleShape shape;
	shape.m_radius = PIXEL_TO_METERS(radius);
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;

	b->CreateFixture(&fixture);

	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);
	pbody->width = pbody->height = radius;

	return pbody;
}

PhysBody* ModulePhysics::CreateRectangle(int x, int y, int width, int height, float angle, b2BodyType Type)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = Type;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);
	body.angle = angle * DEG2RAD;

	b2Body* b = world->CreateBody(&body);
	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.restitution = 0.01f;

	b->CreateFixture(&fixture);

	pbody->body = b;
	pbody->width = (int)(width * 0.5f);
	pbody->height = (int)(height * 0.5f);

	return pbody;
}

PhysBody* ModulePhysics::CreateChain(int x, int y, const int* points, int size)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_staticBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2ChainShape shape;
	b2Vec2* p = new b2Vec2[size / 2];

	for (int i = 0; i < size / 2; ++i)
	{
		p[i].x = PIXEL_TO_METERS(points[i * 2 + 0]);
		p[i].y = PIXEL_TO_METERS(points[i * 2 + 1]);
	}

	shape.CreateLoop(p, size / 2);

	b2FixtureDef fixture;
	fixture.shape = &shape;

	b->CreateFixture(&fixture);

	delete p;

	pbody->body = b;
	pbody->width = pbody->height = 0;

	return pbody;
}

void PhysBody::GetPosition(int& x, int &y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x) - (width);
	y = METERS_TO_PIXELS(pos.y) - (height);
}

void PhysBody::GetPhysicPosition(int& x, int& y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x);
	y = METERS_TO_PIXELS(pos.y);
}

float PhysBody::GetRotation() const
{
	return body->GetAngle();
}

bool PhysBody::Contains(int x, int y) const
{
	b2Vec2 p(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	const b2Fixture* fixture = body->GetFixtureList();

	while(fixture != NULL)
	{
		if(fixture->GetShape()->TestPoint(body->GetTransform(), p) == true)
			return true;
		fixture = fixture->GetNext();
	}

	return false;
}

int PhysBody::RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const
{
	int ret = -1;

	b2RayCastInput input;
	b2RayCastOutput output;

	input.p1.Set(PIXEL_TO_METERS(x1), PIXEL_TO_METERS(y1));
	input.p2.Set(PIXEL_TO_METERS(x2), PIXEL_TO_METERS(y2));
	input.maxFraction = 1.0f;

	const b2Fixture* fixture = body->GetFixtureList();

	while(fixture != NULL)
	{
		if(fixture->GetShape()->RayCast(&output, input, body->GetTransform(), 0) == true)
		{
			// do we want the normal ?

			float fx = (float)(x2 - x1);
			float fy = (float)(y2 - y1);
			float dist = sqrtf((fx*fx) + (fy*fy));

			normal_x = output.normal.x;
			normal_y = output.normal.y;

			return (int)(output.fraction * dist);
		}
		fixture = fixture->GetNext();
	}

	return ret;
}

void ModulePhysics::BeginContact(b2Contact* contact)
{
	b2BodyUserData dataA = contact->GetFixtureA()->GetBody()->GetUserData();
	b2BodyUserData dataB = contact->GetFixtureB()->GetBody()->GetUserData();

	PhysBody* physA = (PhysBody*)dataA.pointer;
	PhysBody* physB = (PhysBody*)dataB.pointer;

	LOG("Begin contact between bodies");

	if (physA && physA->listener != NULL)
	{
		LOG("Contact with body A");
		physA->listener->OnCollision(physA, physB);
	}

	if (physB && physB->listener != NULL)
	{
		LOG("Contact with body B");
		physB->listener->OnCollision(physB, physA);
	}
}
PhysBody* ModulePhysics::CreateRectangleSensor(int x, int y, int width, int height)
{
    PhysBody* pbody = new PhysBody();
    pbody->isActive = true;

    b2BodyDef body;
    body.type = b2_staticBody;
    body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
    body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

    b2Body* b = world->CreateBody(&body);
    b2PolygonShape box;
    box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

    b2FixtureDef fixture;
    fixture.shape = &box;
    fixture.isSensor = true; // Configurar como sensor

    b->CreateFixture(&fixture);

    pbody->body = b;
    pbody->width = width;
    pbody->height = height;

    return pbody;
}
std::vector<b2Vec2> PhysBody::GetVertices() const
{
	std::vector<b2Vec2> vertices; // Vector para almacenar los vértices

	if (body == nullptr)
		return vertices;

	// Iterar sobre las fixtures del cuerpo
	for (b2Fixture* fixture = body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext())
	{
		const b2Shape* shape = fixture->GetShape();

		// Comprobar si la shape es del tipo b2ChainShape
		if (shape->GetType() == b2Shape::e_chain)
		{
			const b2ChainShape* chainShape = static_cast<const b2ChainShape*>(shape);

			// Recorrer los vértices del shape
			for (int i = 0; i < chainShape->m_count; ++i)
			{
				vertices.emplace_back(chainShape->m_vertices[i]); // Añadir vértices al vector
			}
		}
	}

	return vertices;
}
