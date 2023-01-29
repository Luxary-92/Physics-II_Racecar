#include "Globals.h"
#include "Application.h"
#include "ModulePlayer.h"
#include "Primitive.h"
#include "PhysVehicle3D.h"
#include "PhysBody3D.h"
#include "ModulePhysics3D.h"
#include "ModuleCamera3D.h"


ModulePlayer::ModulePlayer(Application* app, bool start_enabled) : Module(app, start_enabled), vehicle_car(NULL)
{
	turn = acceleration = brake = 0.0f;
}

ModulePlayer::~ModulePlayer()
{}

// Load assets
bool ModulePlayer::Start()
{
	LOG("Loading player");

	VehicleInfo car;

	// Car properties ----------------------------------------
	car.chassis_size.Set(2.5, 1.3, 4);
	car.chassis_offset.Set(0, 1, 0);
	car.mass = 500.0f;
	car.suspensionStiffness = 15.88f;
	car.suspensionCompression = 0.83f;
	car.suspensionDamping = 0.88f;
	car.maxSuspensionTravelCm = 1000.0f;
	car.frictionSlip = 50.5;
	car.maxSuspensionForce = 6000.0f;

	// Wheel properties ---------------------------------------
	float connection_height = 1.2f;
	float wheel_radius = 0.6f;
	float wheel_width = 0.5f;
	float suspensionRestLength = 1.2f;

	// Life properties ---------------------------------------
	float life_connection_height = 2.0f;
	float life_x = 0.83f;
	float life_y = 0.5f;
	float life_z = 0.5;
	float life_suspensionRestLength = 1.2f;

	// Don't change anything below this line ------------------

	float half_width = car.chassis_size.x*0.5f;
	float half_length = car.chassis_size.z*0.5f;
	
	vec3 direction(0,-1,0);
	vec3 axis(-1,0,0);
	
	car.num_wheels = 4;
	car.wheels = new Wheel[4];

	car.num_lifes = 3;
	car.lifes = new Lifes[3];

	//Life L
	car.lifes[0].active = true;
	car.lifes[0].life_offset.Set(-1, 1.8, -2);
	car.lifes[0].size_life.x = life_x;
	car.lifes[0].size_life.y = life_y;
	car.lifes[0].size_life.z = life_z;
	//Life M
	car.lifes[1].active = true;
	car.lifes[1].life_offset.Set(0, 1.8, -2);
	car.lifes[1].size_life.x = life_x;
	car.lifes[1].size_life.y = life_y;
	car.lifes[1].size_life.z = life_z;
	//Life M
	car.lifes[2].active = true;
	car.lifes[2].life_offset.Set(+0.835, 1.8, -2);
	car.lifes[2].size_life.x = life_x;
	car.lifes[2].size_life.y = life_y;
	car.lifes[2].size_life.z = life_z;


	// FRONT-LEFT ------------------------
	car.wheels[0].connection.Set(half_width + 0.1f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[0].direction = direction;
	car.wheels[0].axis = axis;
	car.wheels[0].suspensionRestLength = suspensionRestLength;
	car.wheels[0].radius = wheel_radius;
	car.wheels[0].width = wheel_width;
	car.wheels[0].front = true;
	car.wheels[0].drive = true;
	car.wheels[0].brake = false;
	car.wheels[0].steering = true;

	// FRONT-RIGHT ------------------------
	car.wheels[1].connection.Set(-half_width - 0.1f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[1].direction = direction;
	car.wheels[1].axis = axis;
	car.wheels[1].suspensionRestLength = suspensionRestLength;
	car.wheels[1].radius = wheel_radius;
	car.wheels[1].width = wheel_width;
	car.wheels[1].front = true;
	car.wheels[1].drive = true;
	car.wheels[1].brake = false;
	car.wheels[1].steering = true;

	// REAR-LEFT ------------------------
	car.wheels[2].connection.Set(half_width + 0.3f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[2].direction = direction;
	car.wheels[2].axis = axis;
	car.wheels[2].suspensionRestLength = suspensionRestLength;
	car.wheels[2].radius = wheel_radius;
	car.wheels[2].width = wheel_width;
	car.wheels[2].front = false;
	car.wheels[2].drive = false;
	car.wheels[2].brake = true;
	car.wheels[2].steering = false;

	// REAR-RIGHT ------------------------
	car.wheels[3].connection.Set(-half_width - 0.3f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[3].direction = direction;
	car.wheels[3].axis = axis;
	car.wheels[3].suspensionRestLength = suspensionRestLength;
	car.wheels[3].radius = wheel_radius;
	car.wheels[3].width = wheel_width;
	car.wheels[3].front = false;
	car.wheels[3].drive = false;
	car.wheels[3].brake = true;
	car.wheels[3].steering = false;

	vehicle_car = App->physics->AddVehicle(car);
	vehicle_car->SetPos(0, 12, -10);
	
	return true;
}

// Unload assets
bool ModulePlayer::CleanUp()
{
	LOG("Unloading player");

	return true;
}

// Update: draw background
update_status ModulePlayer::Update(float dt)
{
	turn = acceleration = brake = 0.0f;

	if(App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
	{
		acceleration = MAX_ACCELERATION;
	}

	if(App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
	{
		if(turn < TURN_DEGREES)
			turn +=  TURN_DEGREES;
	}

	if(App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
	{
		if(turn > -TURN_DEGREES)
			turn -= TURN_DEGREES;
	}

	if(App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
	{
		brake = BRAKE_POWER;
	}

	if (App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN)
	{
		vehicle_car->info.lifes->active = !vehicle_car->info.lifes->active;
	}

	vehicle_car->ApplyEngineForce(acceleration);
	vehicle_car->Turn(turn);
	vehicle_car->Brake(brake);

	vehicle_car->Render();

	char title[80];
	sprintf_s(title, "%.1f Km/h", vehicle_car->GetKmh());
	App->window->SetTitle(title);

	//Camera Position
	//vec3 posicion = vec3(vehicle->vehicle->getChassisWorldTransform().getOrigin().getX(),
	//					 vehicle->vehicle->getChassisWorldTransform().getOrigin().getY(),
	//					 vehicle->vehicle->getChassisWorldTransform().getOrigin().getZ());
	//App->camera->Look(vec3(posicion.x, posicion.y + 10, posicion.z + 10), posicion);




	return UPDATE_CONTINUE;
}



