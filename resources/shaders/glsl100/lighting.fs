#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;

uniform vec4 colDiffuse;


// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;
uniform float fogDensity;

void main()
{

    vec2 uvs = fragTexCoord;
    uvs.x = 1.0 - uvs.x;
    uvs.y = 1.0 - uvs.y;
    vec4 texelColor = texture2D(texture0, uvs);   
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);
    vec4 tint = colDiffuse * fragColor;


    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPosition);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*NdotL;

            float specCo = 0.0;
            if (NdotL > 0.0) specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine
            specular += specCo;
        }
    }

    vec4 finalColor = (texelColor*((tint + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    finalColor += texelColor*(ambient/10.0);
    finalColor = pow(finalColor, vec4(1.0 / 2.2));

    float dist = length(viewPos - fragPosition);
    const vec4 fogColor = vec4(0,0,0, 1.0);
    float fogFactor = 1.0/exp((dist*fogDensity)*(dist*fogDensity));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    finalColor = mix(fogColor, finalColor, fogFactor);
    finalColor.a = texelColor.a;

    gl_FragColor = finalColor;

}
