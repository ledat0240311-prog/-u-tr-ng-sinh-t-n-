#ifndef RENDERER3D_H
#define RENDERER3D_H

#include <GLES3/gl3.h>
#include "Math3D.h"
#include <vector>

namespace LastSurvivor {

struct Mat4 {
    float m[16];

    static Mat4 Identity();
    static Mat4 Perspective(float fovYDegrees, float aspect, float zNear, float zFar);
    static Mat4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up);
    static Mat4 Translate(float tx, float ty, float tz);
    static Mat4 RotateY(float degrees);
    static Mat4 RotateX(float degrees);
    static Mat4 Scale(float sx, float sy, float sz);
    static Mat4 Multiply(const Mat4& a, const Mat4& b);
};

class Renderer3D {
public:
    Renderer3D();
    ~Renderer3D();

    bool Init();
    void Resize(int width, int height);

    void BeginFrame(const Mat4& viewMatrix, const Mat4& projMatrix);
    void DrawCube(const Vector3& pos, const Vector3& scale, float yaw, float r, float g, float b);
    void DrawGround(float size);
    void DrawLine(const Vector3& start, const Vector3& end, float r, float g, float b, float width);
    void EndFrame();

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

private:
    GLuint CreateShader(GLenum type, const char* source);
    GLuint CreateProgram(const char* vertSrc, const char* fragSrc);

    int m_Width{1280};
    int m_Height{720};

    GLuint m_Program{0};
    GLint m_LocMVP{-1};
    GLint m_LocModel{-1};
    GLint m_LocColor{-1};
    GLint m_LocLightDir{-1};

    GLuint m_CubeVBO{0};
    GLuint m_CubeVAO{0};

    GLuint m_GroundVBO{0};
    GLuint m_GroundVAO{0};

    GLuint m_LineVBO{0};
    GLuint m_LineVAO{0};

    Mat4 m_ViewMatrix;
    Mat4 m_ProjMatrix;
};

} // namespace LastSurvivor

#endif // RENDERER3D_H
