#version 330 core

layout(location = 0) in vec3 vertVertex;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertTex;

uniform int morphingParam;
uniform vec3 spotlightPosition;
uniform vec3 spotlightDirection;
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat4 normalMat;

out vec3 fNormal;
out vec3 fPosition;
out vec2 fTex;
out vec3 fLightRelativeDirection;
out vec3 fSpotLightDirection;


float help_func(float a_sqrt, float b_sqrt){
    return sqrt(1 - a_sqrt / 2 - b_sqrt / 2 + a_sqrt * b_sqrt / 3);
}


//link to formula https://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html

vec4 spherify(vec3 vertex) {

    vec3 result = vec3(help_func(vertex.y * vertex.y, vertex.z*vertex.z),
                       help_func(vertex.z*vertex.z, vertex.x*vertex.x),
                       help_func(vertex.x*vertex.x, vertex.y*vertex.y));
    result = result + (1-result) / 100 * morphingParam;

    return vec4(result*vertex,1);

}

 float get_scale() {
 	vec3 scaleFactors = vec3(
 		length(modelMat[0].xyz),
 		length(modelMat[1].xyz),
 		length(modelMat[2].xyz)
 	);
 	return 1 / max(max(scaleFactors.x, scaleFactors.y), scaleFactors.z);
 }



void main() {
    vec3 result = vec3(help_func(vertVertex.y * vertVertex.y, vertVertex.z*vertVertex.z),
                       help_func(vertVertex.z*vertVertex.z, vertVertex.x*vertVertex.x),
                       help_func(vertVertex.x*vertVertex.x, vertVertex.y*vertVertex.y));

    result = result + (1-result) / 100 * morphingParam;

    vec4 vertex = vec4(result*vertVertex, 1);

    vec4 temp = vec4(vertNormal, 1);
    temp = normalize(vertex) + (temp - normalize(vertex)) / 100 * morphingParam;

    fNormal = normalize(mat3(normalMat) * temp.xyz);
    fPosition = vec3(modelMat * vertex); //vertVertex;
    fTex = vertTex;

    fSpotLightDirection = mat3(viewMat) * spotlightDirection;
    fLightRelativeDirection = mat3(viewMat) * ( vertex.xyz - spotlightPosition);

    gl_Position = projMat * viewMat * modelMat * vertex;
}
