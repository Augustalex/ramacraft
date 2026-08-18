#include "Shader.hpp"
#include <iostream>
#include <vector>

Shader::~Shader() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

GLuint Shader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* srcPtr = source.c_str();
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        std::cerr << "Shader compile error (" << (type == GL_VERTEX_SHADER ? "VERT" : "FRAG") << "):\n"
                  << log.data() << "\nSource:\n" << source << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::loadFromSource(const std::string& vertSrc, const std::string& fragSrc) {
#ifdef __EMSCRIPTEN__
    std::string header = "#version 300 es\nprecision highp float;\n";
#else
    std::string header = "#version 330 core\n";
#endif

    std::string fullVert = header + vertSrc;
    std::string fullFrag = header + fragSrc;

    GLuint vert = compileShader(GL_VERTEX_SHADER, fullVert);
    if (!vert) return false;

    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fullFrag);
    if (!frag) {
        glDeleteShader(vert);
        return false;
    }

    if (m_program != 0) {
        glDeleteProgram(m_program);
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);

    GLint linked = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint logLen = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetProgramInfoLog(m_program, logLen, nullptr, log.data());
        std::cerr << "Shader program link error:\n" << log.data() << std::endl;
        glDeleteProgram(m_program);
        m_program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    return m_program != 0;
}

void Shader::use() const {
    if (m_program != 0) {
        glUseProgram(m_program);
    }
}

void Shader::unbind() const {
    glUseProgram(0);
}

GLint Shader::getUniformLocation(const std::string& name) const {
    if (m_program == 0) return -1;
    return glGetUniformLocation(m_program, name.c_str());
}

void Shader::setMat4(const std::string& name, const Mat4& mat) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, mat.m);
    }
}

void Shader::setVec2(const std::string& name, const Vec2& v) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform2f(loc, v.x, v.y);
    }
}

void Shader::setVec3(const std::string& name, const Vec3& v) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform3f(loc, v.x, v.y, v.z);
    }
}

void Shader::setVec4(const std::string& name, const Vec4& v) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform4f(loc, v.x, v.y, v.z, v.w);
    }
}

void Shader::setFloat(const std::string& name, float v) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1f(loc, v);
    }
}

void Shader::setInt(const std::string& name, int v) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1i(loc, v);
    }
}

// -------------------------------------------------------------
// Shader Source Definitions
// -------------------------------------------------------------

namespace Shaders {

const std::string VOXEL_VERT = R"(
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aAO;
layout (location = 4) in float aLight;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uPlayerPos;
uniform float uCylinderRadius;
uniform float uCurvatureEnable;

out vec3 vWorldPos;
out vec2 vTexCoord;
out vec3 vNormal;
out float vAO;
out float vBlockLight;
out float vDistToPlayer;

const float PI = 3.14159265358979323846;
const float CIRCUMFERENCE = 1536.0;
const float R = 244.46199; // 1536.0 / (2.0 * PI) (2x colossal Rama radius)

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    
    // True 360-Degree Closed Rama Cylinder Geometry
    if (uCurvatureEnable > 0.5) {
        float theta = worldPos.x * (2.0 * PI / CIRCUMFERENCE);
        float r = R - worldPos.y;
        
        vec3 curPos;
        curPos.x = sin(theta) * r;
        curPos.y = R - cos(theta) * r;
        curPos.z = worldPos.z;
        
        // Rotate normal around Z axis by theta so lighting is 100% physically accurate
        vec3 curNorm;
        curNorm.x = aNormal.x * cos(theta) + aNormal.y * sin(theta);
        curNorm.y = -aNormal.x * sin(theta) + aNormal.y * cos(theta);
        curNorm.z = aNormal.z;
        
        worldPos.xyz = curPos;
        vNormal = normalize(curNorm);
    } else {
        vNormal = aNormal;
    }

    vWorldPos = worldPos.xyz;
    vTexCoord = aTexCoord;
    vAO = aAO;
    vBlockLight = aLight;
    vDistToPlayer = length(worldPos.xyz - uPlayerPos);

    gl_Position = uProjection * uView * worldPos;
}
)";

const std::string VOXEL_FRAG = R"(
precision highp float;

in vec3 vWorldPos;
in vec2 vTexCoord;
in vec3 vNormal;
in float vAO;
in float vBlockLight;
in float vDistToPlayer;

uniform sampler2D uAtlas;
uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform float uSunIntensity;
uniform vec3 uAmbientColor;
uniform vec3 uFogColor;
uniform float uFogDensity;

// Dynamic Flashlight
uniform vec3 uFlashlightPos;
uniform vec3 uFlashlightDir;
uniform vec3 uFlashlightColor;
uniform float uFlashlightEnable;

// Dynamic Point Lights (Torches, Lasers, Biots, Reactors)
#define MAX_POINT_LIGHTS 16
uniform int uNumPointLights;
uniform vec3 uPointLightPos[MAX_POINT_LIGHTS];
uniform vec3 uPointLightColor[MAX_POINT_LIGHTS];
uniform float uPointLightRadius[MAX_POINT_LIGHTS];

out vec4 FragColor;

void main() {
    vec4 texColor = texture(uAtlas, vTexCoord);
    if (texColor.a < 0.1) discard;

    vec3 norm = normalize(vNormal);

    // 1. Rama Linear Sun / Day Lighting
    // Directional light from linear sun strips along Rama cylinder ceiling
    float NdotL = max(dot(norm, uSunDir), 0.0);
    vec3 sunLight = uSunColor * (NdotL * uSunIntensity);

    // 2. Ambient Occlusion + Base Ambient
    float aoFactor = mix(0.4, 1.0, vAO);
    vec3 ambient = uAmbientColor * aoFactor;

    // 3. Dynamic Flashlight (Spotlight cone)
    vec3 flashLight = vec3(0.0);
    if (uFlashlightEnable > 0.5) {
        vec3 toLight = uFlashlightPos - vWorldPos;
        float dist = length(toLight);
        if (dist < 40.0) {
            vec3 lightDir = normalize(toLight);
            float spotCos = dot(-lightDir, normalize(uFlashlightDir));
            float cutOff = 0.88; // ~30 degree cone
            float outerCutOff = 0.80; // soft edge
            if (spotCos > outerCutOff) {
                float intensity = clamp((spotCos - outerCutOff) / (cutOff - outerCutOff), 0.0, 1.0);
                float atten = 1.0 / (1.0 + 0.06 * dist + 0.015 * dist * dist);
                float nDot = max(dot(norm, lightDir), 0.0);
                flashLight = uFlashlightColor * (intensity * atten * (nDot + 0.15) * 2.2);
            }
        }
    }

    // 4. Dynamic Point Lights (Torches, Crystals, Lasers)
    vec3 pointLights = vec3(0.0);
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        if (i >= uNumPointLights) break;
        vec3 toLight = uPointLightPos[i] - vWorldPos;
        float dist = length(toLight);
        float radius = uPointLightRadius[i];
        if (dist < radius) {
            vec3 lightDir = normalize(toLight);
            float atten = clamp(1.0 - (dist / radius), 0.0, 1.0);
            atten = atten * atten;
            float nDot = max(dot(norm, lightDir), 0.0) + 0.2;
            pointLights += uPointLightColor[i] * (atten * nDot * 1.8);
        }
    }

    // 5. Total Combined Lighting
    vec3 totalLight = ambient + sunLight + flashLight + pointLights;
    
    // Add block's internal luminescence (e.g. Cobalt crystals, reactors)
    totalLight += vec3(vBlockLight);

    vec3 finalColor = texColor.rgb * totalLight;

    // 6. Volumetric Rama Atmosphere Fog (Dark cylinder depth)
    float fogFactor = 1.0 - exp(-pow(vDistToPlayer * uFogDensity, 1.5));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    finalColor = mix(finalColor, uFogColor, fogFactor);

    FragColor = vec4(finalColor, texColor.a);
}
)";

const std::string ENTITY_VERT = R"(
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uPlayerPos;
uniform float uCylinderRadius;
uniform float uCurvatureEnable;

out vec3 vWorldPos;
out vec2 vTexCoord;
out vec3 vNormal;
out float vDistToPlayer;

const float PI = 3.14159265358979323846;
const float CIRCUMFERENCE = 1536.0;
const float R = 244.46199;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    
    if (uCurvatureEnable > 0.5) {
        float theta = worldPos.x * (2.0 * PI / CIRCUMFERENCE);
        float r = R - worldPos.y;
        
        vec3 curPos;
        curPos.x = sin(theta) * r;
        curPos.y = R - cos(theta) * r;
        curPos.z = worldPos.z;
        
        vec3 n = mat3(uModel) * aNormal;
        vec3 curNorm;
        curNorm.x = n.x * cos(theta) + n.y * sin(theta);
        curNorm.y = -n.x * sin(theta) + n.y * cos(theta);
        curNorm.z = n.z;
        
        worldPos.xyz = curPos;
        vNormal = normalize(curNorm);
    } else {
        vNormal = mat3(uModel) * aNormal;
    }

    vWorldPos = worldPos.xyz;
    vTexCoord = aTexCoord;
    vDistToPlayer = length(worldPos.xyz - uPlayerPos);

    gl_Position = uProjection * uView * worldPos;
}
)";

const std::string ENTITY_FRAG = R"(
precision highp float;

in vec3 vWorldPos;
in vec2 vTexCoord;
in vec3 vNormal;
in float vDistToPlayer;

uniform sampler2D uAtlas;
uniform vec4 uTint;
uniform vec3 uSunColor;
uniform float uSunIntensity;
uniform vec3 uAmbientColor;
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform vec3 uFlashlightPos;
uniform vec3 uFlashlightDir;
uniform vec3 uFlashlightColor;
uniform float uFlashlightEnable;

out vec4 FragColor;

void main() {
    vec4 texColor = texture(uAtlas, vTexCoord) * uTint;
    if (texColor.a < 0.1) discard;

    vec3 norm = normalize(vNormal);
    float NdotL = max(dot(norm, vec3(0.0, 1.0, 0.0)), 0.0);
    vec3 light = uAmbientColor + uSunColor * (NdotL * uSunIntensity);

    if (uFlashlightEnable > 0.5) {
        vec3 toLight = uFlashlightPos - vWorldPos;
        float dist = length(toLight);
        if (dist < 40.0) {
            vec3 lightDir = normalize(toLight);
            float spotCos = dot(-lightDir, normalize(uFlashlightDir));
            if (spotCos > 0.80) {
                float intensity = clamp((spotCos - 0.80) / 0.08, 0.0, 1.0);
                float atten = 1.0 / (1.0 + 0.06 * dist + 0.015 * dist * dist);
                light += uFlashlightColor * (intensity * atten * 2.0);
            }
        }
    }

    vec3 finalColor = texColor.rgb * light;
    float fogFactor = clamp(1.0 - exp(-pow(vDistToPlayer * uFogDensity, 1.5)), 0.0, 1.0);
    finalColor = mix(finalColor, uFogColor, fogFactor);

    FragColor = vec4(finalColor, texColor.a);
}
)";

const std::string UI_VERT = R"(
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

uniform mat4 uProjection;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vTexCoord = aTexCoord;
    vColor = aColor;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

const std::string UI_FRAG = R"(
precision highp float;

in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uAtlas;
uniform int uUseTexture;

out vec4 FragColor;

void main() {
    if (uUseTexture == 1 && vTexCoord.x >= 0.0) {
        vec4 texColor = texture(uAtlas, vTexCoord);
        FragColor = texColor * vColor;
    } else {
        FragColor = vColor;
    }
}
)";

} // namespace Shaders
