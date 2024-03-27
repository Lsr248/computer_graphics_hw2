#version 330 core

uniform sampler2D tex_2d;
uniform bool enableDirectionalLight;
uniform bool enableSpotLight;
uniform float spotLightAngle;

in vec3 fNormal;
in vec3 fPosition;
in vec2 fTex;
in vec3 fLightRelativeDirection;
in vec3 fSpotLightDirection;


out vec4 color;

vec4 get_ambient() {
    const vec4 ambientLightColor = vec4(1.0, 1.0, 1.0, 1.0);
    const float ambientStength = 0.2;
    return ambientLightColor * ambientStength;
}

vec4 get_spot() {

    if (enableSpotLight) {
        float cos = dot(normalize(fLightRelativeDirection), normalize(fSpotLightDirection));
        if (cos > 0 && acos(cos) < radians(spotLightAngle)){
            return vec4(1, 1, 0.7, 1);
        }
        return vec4(0, 0, 0, 0);
    }
    return vec4(0, 0, 0, 0);

}

vec4 get_directional() {
 if (enableDirectionalLight) {
        const vec4 diffuseLightColor = vec4(1.0, 0.92, 0.5, 1.0); // RGBA Color
        const vec3 diffuseLightSource = vec3(3.0, 5.0, 1.0);      // Position in World space
        vec3 diffuseLightDirection = normalize(diffuseLightSource - fPosition);
        return max(dot(fNormal, diffuseLightDirection), 0) * diffuseLightColor;
    }
    return vec4(0, 0, 0, 0);
}

void main() {

    vec4 color_dir = get_directional() ;
    vec4 color_spot = get_spot();

    color += (color_dir+color_spot + get_ambient()) * texture(tex_2d, fTex);

}
