#include "Renderer3D.h"
#include <android/log.h>
#include <cmath>
#include <cstring>

#define LOG_TAG "LastSurvivorGL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace LastSurvivor {

constexpr float DEG2RAD = 3.14159265358979323846f / 180.0f;

Mat4 Mat4::Identity() {
    Mat4 res{};
    res.m[0] = 1.0f; res.m[5] = 1.0f; res.m[10] = 1.0f; res.m[15] = 1.0f;
    return res;
}

Mat4 Mat4::Perspective(float fovYDegrees, float aspect, float zNear, float zFar) {
    Mat4 res{};
    float tanHalfFov = std::tan((fovYDegrees * 0.5f) * DEG2RAD);
    res.m[0] = 1.0f / (aspect * tanHalfFov);
    res.m[5] = 1.0f / tanHalfFov;
    res.m[10] = -(zFar + zNear) / (zFar - zNear);
    res.m[11] = -1.0f;
    res.m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
    return res;
}

Mat4 Mat4::LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    Vector3 f = (target - eye).Normalized();
    Vector3 upNorm = up.Normalized();

    // r = f x up
    Vector3 r(
        f.y * upNorm.z - f.z * upNorm.y,
        f.z * upNorm.x - f.x * upNorm.z,
        f.x * upNorm.y - f.y * upNorm.x
    );
    r = r.Normalized();

    // u = r x f
    Vector3 u(
        r.y * f.z - r.z * f.y,
        r.z * f.x - r.x * f.z,
        r.x * f.y - r.y * f.x
    );

    Mat4 res = Mat4::Identity();
    res.m[0] = r.x;  res.m[4] = r.y;  res.m[8]  = r.z;  res.m[12] = -Vector3::Dot(r, eye);
    res.m[1] = u.x;  res.m[5] = u.y;  res.m[9]  = u.z;  res.m[13] = -Vector3::Dot(u, eye);
    res.m[2] = -f.x; res.m[6] = -f.y; res.m[10] = -f.z; res.m[14] = Vector3::Dot(f, eye);
    res.m[15] = 1.0f;
    return res;
}

Mat4 Mat4::Translate(float tx, float ty, float tz) {
    Mat4 res = Mat4::Identity();
    res.m[12] = tx;
    res.m[13] = ty;
    res.m[14] = tz;
    return res;
}

Mat4 Mat4::RotateY(float degrees) {
    Mat4 res = Mat4::Identity();
    float rad = degrees * DEG2RAD;
    float c = std::cos(rad);
    float s = std::sin(rad);
    res.m[0] = c;  res.m[8] = s;
    res.m[2] = -s; res.m[10] = c;
    return res;
}

Mat4 Mat4::RotateX(float degrees) {
    Mat4 res = Mat4::Identity();
    float rad = degrees * DEG2RAD;
    float c = std::cos(rad);
    float s = std::sin(rad);
    res.m[5] = c;  res.m[9] = -s;
    res.m[6] = s;  res.m[10] = c;
    return res;
}

Mat4 Mat4::Scale(float sx, float sy, float sz) {
    Mat4 res = Mat4::Identity();
    res.m[0] = sx;
    res.m[5] = sy;
    res.m[10] = sz;
    return res;
}

Mat4 Mat4::Multiply(const Mat4& a, const Mat4& b) {
    Mat4 res{};
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a.m[k * 4 + row] * b.m[col * 4 + k];
            }
            res.m[col * 4 + row] = sum;
        }
    }
    return res;
}

// Shader definitions for OpenGL ES 3.0
static const char* VERTEX_SHADER_SRC =
    "#version 300 es\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "uniform mat4 uMVP;\n"
    "uniform mat4 uModel;\n"
    "out vec3 vNormal;\n"
    "out vec3 vWorldPos;\n"
    "void main() {\n"
    "    vNormal = mat3(uModel) * aNormal;\n"
    "    vec4 worldPos = uModel * vec4(aPos, 1.0);\n"
    "    vWorldPos = worldPos.xyz;\n"
    "    gl_Position = uMVP * vec4(aPos, 1.0);\n"
    "}\n";

static const char* FRAGMENT_SHADER_SRC =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec3 vNormal;\n"
    "in vec3 vWorldPos;\n"
    "uniform vec4 uColor;\n"
    "uniform vec3 uLightDir;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    vec3 norm = normalize(vNormal);\n"
    "    vec3 light = normalize(-uLightDir);\n"
    "    float diff = max(dot(norm, light), 0.0);\n"
    "    float ambient = 0.35;\n"
    "    vec3 litColor = uColor.rgb * (ambient + diff * 0.65);\n"
    "    FragColor = vec4(litColor, uColor.a);\n"
    "}\n";

Renderer3D::Renderer3D() = default;

Renderer3D::~Renderer3D() {
    if (m_Program != 0) glDeleteProgram(m_Program);
    if (m_CubeVBO != 0) glDeleteBuffers(1, &m_CubeVBO);
    if (m_CubeVAO != 0) glDeleteVertexArrays(1, &m_CubeVAO);
    if (m_GroundVBO != 0) glDeleteBuffers(1, &m_GroundVBO);
    if (m_GroundVAO != 0) glDeleteVertexArrays(1, &m_GroundVAO);
    if (m_LineVBO != 0) glDeleteBuffers(1, &m_LineVBO);
    if (m_LineVAO != 0) glDeleteVertexArrays(1, &m_LineVAO);
}

GLuint Renderer3D::CreateShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char info[512];
        glGetShaderInfoLog(shader, sizeof(info), nullptr, info);
        LOGE("Shader compilation error: %s", info);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint Renderer3D::CreateProgram(const char* vertSrc, const char* fragSrc) {
    GLuint vs = CreateShader(GL_VERTEX_SHADER, vertSrc);
    GLuint fs = CreateShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!vs || !fs) return 0;

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint linked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        char info[512];
        glGetProgramInfoLog(prog, sizeof(info), nullptr, info);
        LOGE("Program link error: %s", info);
        glDeleteProgram(prog);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

bool Renderer3D::Init() {
    m_Program = CreateProgram(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
    if (!m_Program) {
        LOGE("Failed to create GLES3 shader program");
        return false;
    }

    m_LocMVP = glGetUniformLocation(m_Program, "uMVP");
    m_LocModel = glGetUniformLocation(m_Program, "uModel");
    m_LocColor = glGetUniformLocation(m_Program, "uColor");
    m_LocLightDir = glGetUniformLocation(m_Program, "uLightDir");

    // 36 vertices with normals for 3D Cube
    float cubeVertices[] = {
        // Front face (Z = +0.5, normal 0, 0, 1)
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

        // Back face (Z = -0.5, normal 0, 0, -1)
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

        // Top face (Y = +0.5, normal 0, 1, 0)
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,

        // Bottom face (Y = -0.5, normal 0, -1, 0)
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,

        // Right face (X = +0.5, normal 1, 0, 0)
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,

        // Left face (X = -0.5, normal -1, 0, 0)
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f
    };

    glGenVertexArrays(1, &m_CubeVAO);
    glGenBuffers(1, &m_CubeVBO);
    glBindVertexArray(m_CubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Ground Plane quad
    float groundVertices[] = {
        -40.0f, 0.0f, -40.0f,   0.0f, 1.0f, 0.0f,
         40.0f, 0.0f, -40.0f,   0.0f, 1.0f, 0.0f,
         40.0f, 0.0f,  40.0f,   0.0f, 1.0f, 0.0f,
         40.0f, 0.0f,  40.0f,   0.0f, 1.0f, 0.0f,
        -40.0f, 0.0f,  40.0f,   0.0f, 1.0f, 0.0f,
        -40.0f, 0.0f, -40.0f,   0.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &m_GroundVAO);
    glGenBuffers(1, &m_GroundVBO);
    glBindVertexArray(m_GroundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_GroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    LOGI("Renderer3D initialized successfully.");
    return true;
}

void Renderer3D::Resize(int width, int height) {
    m_Width = width;
    m_Height = height;
    glViewport(0, 0, width, height);
}

void Renderer3D::BeginFrame(const Mat4& viewMatrix, const Mat4& projMatrix) {
    m_ViewMatrix = viewMatrix;
    m_ProjMatrix = projMatrix;

    // Dark moody post-apocalyptic sky color
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_Program);
    glUniform3f(m_LocLightDir, -0.4f, -1.0f, -0.6f);
}

void Renderer3D::DrawCube(const Vector3& pos, const Vector3& scale, float yaw, float r, float g, float b) {
    Mat4 t = Mat4::Translate(pos.x, pos.y, pos.z);
    Mat4 rot = Mat4::RotateY(yaw);
    Mat4 s = Mat4::Scale(scale.x, scale.y, scale.z);

    Mat4 model = Mat4::Multiply(t, Mat4::Multiply(rot, s));
    Mat4 mv = Mat4::Multiply(m_ViewMatrix, model);
    Mat4 mvp = Mat4::Multiply(m_ProjMatrix, mv);

    glUniformMatrix4fv(m_LocMVP, 1, GL_FALSE, mvp.m);
    glUniformMatrix4fv(m_LocModel, 1, GL_FALSE, model.m);
    glUniform4f(m_LocColor, r, g, b, 1.0f);

    glBindVertexArray(m_CubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Renderer3D::DrawGround(float size) {
    Mat4 model = Mat4::Identity();
    Mat4 mv = Mat4::Multiply(m_ViewMatrix, model);
    Mat4 mvp = Mat4::Multiply(m_ProjMatrix, mv);

    glUniformMatrix4fv(m_LocMVP, 1, GL_FALSE, mvp.m);
    glUniformMatrix4fv(m_LocModel, 1, GL_FALSE, model.m);
    // Dark asphalt / concrete ground color
    glUniform4f(m_LocColor, 0.16f, 0.18f, 0.20f, 1.0f);

    glBindVertexArray(m_GroundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer3D::EndFrame() {
    glBindVertexArray(0);
    glUseProgram(0);
}

} // namespace LastSurvivor
