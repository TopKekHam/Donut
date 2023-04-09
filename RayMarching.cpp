#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <thread>

#include <chrono>
using namespace std::chrono_literals;

#define PI 3.14159265359f

struct vec2 {
	float x, y;

	vec2 operator -(const vec2& other)
	{
		return { x - other.x, y - other.y};
	}

	vec2 operator -()
	{
		return { -x, -y };
	}

	vec2 operator +(const vec2& other)
	{
		return { x + other.x, y + other.y };
	}

	vec2 operator /(const float& f)
	{
		return { x / f, y / f };
	}

	vec2 operator *(const float& f)
	{
		return { x * f, y * f };
	}
};

struct vec3 {
	float x, y, z;

	vec3 operator -(const vec3& other)
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	vec3 operator -()
	{
		return { -x, -y , -z };
	}

	vec3 operator +(const vec3& other)
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	vec3 operator /(const float& f)
	{
		return { x / f, y / f, z / f };
	}

	vec3 operator *(const float& f)
	{
		return { x * f, y * f, z * f };
	}
};

struct ray_hit {
	float distance;
	vec3 normal;
	vec3 position;
};

#define BUFFER_WIDTH 64
#define BUFFER_HEIGHT 32
#define MAX_RAY_DISTANCE 20.0f

char buffer[(BUFFER_WIDTH + 1) * (BUFFER_HEIGHT + 1)];

float time_now = 0;
float mills_from_start = 0;

void draw_buffer()
{
	printf("\033[%d;%dH", 0, 0);
	printf("%s", buffer);
}

float length(vec3 vec)
{
	return sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

vec3 normalize(vec3 vec)
{
	return vec / length(vec);
}

float clamp(float f, float min, float max)
{
	return (f < min ? min : (f > max ? max : f));
}

float dot(vec3 v1, vec3 v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

vec3 rotX(vec3 vec, float angle)
{
	return vec3{
		vec.x,
		vec.y * cosf(angle) - vec.z * -sinf(angle),
		vec.y * sinf(angle) - vec.z * cosf(angle),
	};
}

vec3 rotY(vec3 vec, float angle)
{
	return vec3{
		vec.x * cosf(angle) - vec.z * sinf(angle),
		vec.y,
		vec.x * -sinf(angle) - vec.z * cosf(angle),
	};
}


void set_char(char c, int x, int y)
{
	buffer[x + (BUFFER_WIDTH + 1) * y] = c;
}

float sphere_radius = 1.0f;

float sphere_distance(vec3 pos) {
	return length(pos) - sphere_radius;
}

float torus_radius = 1.5f;
float torus_width = .5f;

float torus_distance(vec3 pos)
{
	float x = sqrtf((pos.x * pos.x) + (pos.z * pos.z)) - torus_radius;
	return sqrtf((x * x) + (pos.y * pos.y)) - torus_width;
}

float get_distance(vec3 pos)
{
	pos = rotY(pos, (time_now * 90.f) / 180.f * PI);
	pos = rotX(pos, 90.f / 180.f * PI);

	return torus_distance(pos);
}

vec3 get_normal(vec3 pos)
{
	float d = get_distance(pos);
	vec3 n = vec3{ d ,d ,d } - vec3{
		get_distance(pos - vec3{.01f, 0, 0}),
		get_distance(pos - vec3{0, .01f, 0}),
		get_distance(pos - vec3{0, 0, .01f}),
	};

	return normalize(n);
}

ray_hit ray_march(vec3 ro, vec3 rd)
{
	float eps = 0.01f;
	int steps_left = 100;
	float distance_walked = 0;
	vec3 normal;
	vec3 pos;

	while (steps_left > 0)
	{

		vec3 rw = ro + (rd * distance_walked);

		float distance_from_surface = get_distance(rw);
		normal = get_normal(rw);
		pos = rw;

		if (distance_from_surface <= eps)
		{
			break;
		}

		distance_walked += distance_from_surface;

		if (distance_walked > MAX_RAY_DISTANCE)
		{
			distance_walked = MAX_RAY_DISTANCE;
			break;
		}

		steps_left--;
	}

	return { distance_walked , normal, pos};
}

char buf[] = ".:+*=#%@";
int len = sizeof(buf) / sizeof(char) - 1;

char get_color(ray_hit hit)
{

	if (hit.distance == MAX_RAY_DISTANCE) return ' ';

	vec3 light_point = { -1, 4, -2};

	vec3 hit_normal = normalize(hit.position - light_point);

	float dif = clamp(dot(hit_normal, hit.normal), 0, 1);

	int idx = dif * len;

	return buf[idx];
}

int main()
{

	for (int y = 0; y < BUFFER_HEIGHT; y++)
	{
		set_char('\n', BUFFER_WIDTH, y);
	}

	while (true)
	{
		time_now = mills_from_start / 1000.f;

		for (int x = 0; x < BUFFER_WIDTH; x++)
			for (int y = 0; y < BUFFER_HEIGHT; y++)
			{
				float dist = 6;
				vec3 ro = {0, 0 , dist };

				float frag_x = BUFFER_WIDTH, frag_y = BUFFER_HEIGHT;
				frag_x = ((float)x - (frag_x / 2.f)) / (frag_x / 2.f) * 1.5;
				frag_y = ((float)y - (frag_y / 2.f)) / (frag_y / 2.f);

				vec3 rd = normalize( -ro + vec3{ frag_x, frag_y, 4 });

				ray_hit hit = ray_march(ro, rd);

				char c = get_color(hit);
				set_char(c, x, y);
			}

		draw_buffer();
	
		std::this_thread::sleep_for(33ms);
		mills_from_start += 33;
	}

	return 0;
}
