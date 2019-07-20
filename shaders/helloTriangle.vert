#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 0) out vec3 fragColour;

float WIDTH = 800;
float HEIGHT = 600;

layout (binding = 0) uniform Uniform {
  vec3 spherePosition;
  float sphereRadius;
} u;


struct Hit {
  bool did_hit;
  float distance;
};

struct Ray {
 vec3 origin;
 vec3 direction;
 float tmin;
 float tmax;
};

Hit check_intersect(Ray ray) {
  vec3 center =  u.spherePosition;
  float radius = u.sphereRadius;
  float radius2 = radius * radius;

  vec3 L = center - ray.origin;
  float tca = dot(L, ray.direction);

  Hit hit;
  hit.did_hit = false;
  hit.distance = 0.0f;

  if (tca < 0)
  {
      return hit;
  } else {
    float d2 = dot(L, L) - tca * tca;
    if (d2 > radius2)
    {
        return hit;
    } else {
      float thc = sqrt(radius2 - d2);
      float t0 = tca - thc;
      float t1 = tca + thc;
      if (t0 < 0.0f)
      {
          // if t0 is negative, let's use t1 instead
          t0 = t1;
          if (t0 < 0)
          {
              // both t0 and t1 are negative
              return hit;
          }
      }

      hit.did_hit = true;
      hit.distance = t0;
      return hit;
    }
  }
}

void main() {
  gl_Position = vec4(inPosition, 0.0, 1.0); // Copy 2D position to 3D + depth
  gl_Position.x = gl_Position.x * (WIDTH / HEIGHT);

  Ray ray;
  ray.origin = vec3(0, 0, 1);

  vec3 direction = normalize(-ray.origin + gl_Position.xyz);
  direction = normalize(vec3(direction.x, direction.y, direction.z));
  ray.direction = direction;

  fragColour = vec3(0, 0, 0);

  // ray-sphere intersection
  Hit hit = check_intersect(ray);
  if (hit.did_hit) {
    fragColour = vec3(1, 1, 1);
  }
}
