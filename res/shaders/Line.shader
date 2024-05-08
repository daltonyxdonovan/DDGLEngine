#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float textureID;

uniform mat4 u_MVP; // Projection matrix for clip space transformation

out vec3 vertPos; // Position of the vertex in clip space

void main() {
  vertPos = position;
  gl_Position = u_MVP * vec4(position, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_Color; // Color of the line

in vec3 vertPos; // Position of the vertex in clip space

out vec4 FragColor; // Fragment color

void main() {
    // Set a threshold for the distance to consider a fragment to be on the line
    float threshold = .5; // Adjust as needed for your scene

    // Calculate the distance from the fragment to the line segments formed by adjacent vertices
    float dist = length(gl_FragCoord.xy - vertPos.xy);

    // Check if the distance is close to zero, indicating that the fragment is on the edge of a line
    if (dist < threshold) {
        FragColor = u_Color; // Set the color for fragments on the line
    } else {
        discard; // Discard fragments that are not on the line
    }
}
