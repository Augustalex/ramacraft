#pragma once

#include <string>
#include <vector>
#include "Math3D.hpp"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool loadFromSource(const std::string& vertSrc, const std::string& fragSrc);
    void use() const;
    void unbind() const;

    GLuint getProgramID() const { return m_program; }

    // Uniform setters
    void setMat4(const std::string& name, const Mat4& mat) const;
    void setVec2(const std::string& name, const Vec2& v) const;
    void setVec3(const std::string& name, const Vec3& v) const;
    void setVec4(const std::string& name, const Vec4& v) const;
    void setFloat(const std::string& name, float v) const;
    void setInt(const std::string& name, int v) const;

private:
    GLuint m_program = 0;
    GLuint compileShader(GLenum type, const std::string& source);
    GLint getUniformLocation(const std::string& name) const;
};

namespace Shaders {
    // Shader sources cross-compatible with OpenGL 3.3 Core and WebGL 2.0 (GLSL 300 es)
    extern const std::string VOXEL_VERT;
    extern const std::string VOXEL_FRAG;

    extern const std::string ENTITY_VERT;
    extern const std::string ENTITY_FRAG;

    extern const std::string UI_VERT;
    extern const std::string UI_FRAG;
}
