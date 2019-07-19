#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 0) out vec3 fragColour;

float WIDTH = 800;
float HEIGHT = 600;

void main() {
  gl_Position = vec4(inPosition, 0.0, 1.0); // Copy 2D position to 3D + depth

  vec3 center = vec3(0, 0, -10);
  float radius = 2.0f;
  float radius2 = radius * radius;

  vec3 origin = vec3(0, 0, 1);
  vec3 direction = normalize(-origin + gl_Position.xyz);
  direction = normalize(vec3(direction.x * (WIDTH / HEIGHT), direction.y, direction.z));

  // ray-sphere intersection
  vec3 L = center - origin;
  float tca = dot(L, direction);
  if (tca < 0)
  {
      fragColour = vec3(0, 0, 0);
  } else {
    float d2 = dot(L, L) - tca * tca;
    if (d2 > radius2)
    {
        fragColour = vec3(0, 0, 0);
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
              fragColour = vec3(0, 0, 0);
          }
      }
      
      fragColour = vec3(1, 1, 1);
    }
  }
}
