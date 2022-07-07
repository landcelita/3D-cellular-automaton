#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include <glad/gl.h>
#include "shaders.h"
#include "event_handler.h"
#include "trans_mats.h"

#define GLFW_DLL
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS        
#define GLM_ENABLE_EXPERIMENTAL  
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "common.h"
#include "CA.h"

static const int LENGTH = 50;

// 頂点クラス
// Vertex class
struct Vertex {
    Vertex(const glm::vec3 &position_, const glm::vec3 &color_)
        : position(position_)
        , color(color_) {
    }

    glm::vec3 position;
    glm::vec3 color;
};

// clang-format off
static const glm::vec3 positions[8] = {
    glm::vec3(-0.02f, -0.02f, -0.02f),
    glm::vec3(+0.02f, -0.02f, -0.02f),
    glm::vec3(-0.02f, +0.02f, -0.02f),
    glm::vec3(-0.02f, -0.02f, +0.02f),
    glm::vec3(+0.02f, +0.02f, -0.02f),
    glm::vec3(-0.02f, +0.02f, +0.02f),
    glm::vec3(+0.02f, -0.02f, +0.02f),
    glm::vec3(+0.02f, +0.02f, +0.02f)
};

static const glm::vec3 colors[6] = {
    glm::vec3(1.0f, 0.0f, 0.0f),  // 赤
    glm::vec3(0.0f, 1.0f, 0.0f),  // 緑
    glm::vec3(0.0f, 0.0f, 1.0f),  // 青
    glm::vec3(1.0f, 1.0f, 0.0f),  // イエロー
    glm::vec3(0.0f, 1.0f, 1.0f),  // シアン
    glm::vec3(1.0f, 0.0f, 1.0f),  // マゼンタ
};

static const unsigned int faces[12][3] = {
    { 7, 4, 1 }, { 7, 1, 6 },
    { 2, 4, 7 }, { 2, 7, 5 },
    { 5, 7, 6 }, { 5, 6, 3 },
    { 4, 2, 0 }, { 4, 0, 1 },
    { 3, 6, 1 }, { 3, 1, 0 },
    { 2, 5, 3 }, { 2, 3, 0 }
};
// clang-format on

// バッファを参照する番号
// Indices for vertex/index buffers
GLuint vaoId;
GLuint vertexBufferId;
GLuint indexBufferId;

// VAOの初期化
// Initialize VAO
void initVAO() {
    // Vertex配列の作成
    // Create vertex array
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    int idx = 0;
    for (int x = 0; x < LENGTH; x++) {
        for (int y = 0; y < LENGTH; y++) {
            for (int z = 0; z < LENGTH; z++) {
                for (int face = 0; face < 6; face++) {
                    for (int i = 0; i < 3; i++) {
                        glm::vec3 v3 = positions[faces[face * 2 + 0][i]];
                        v3.x -= 0.1 * x;
                        v3.y -= 0.1 * y;
                        v3.z -= 0.1 * z;

                        vertices.push_back(Vertex(v3, colors[face]));
                        indices.push_back(idx++);
                    }

                    for (int i = 0; i < 3; i++) {
                        glm::vec3 v3 = positions[faces[face * 2 + 1][i]];
                        v3.x -= 0.1 * x;
                        v3.y -= 0.1 * y;
                        v3.z -= 0.1 * z;

                        vertices.push_back(Vertex(v3, colors[face]));
                        indices.push_back(idx++);
                    }
                }
            }
        }
    }

    // VAOの作成
    // Create VAO
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);

    // 頂点バッファオブジェクトの作成
    // Create vertex buffer object
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // 頂点バッファに対する属性情報の設定
    // Setup attributes for vertex buffer object
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    // 頂点番号バッファオブジェクトの作成
    // Create index buffer object
    glGenBuffers(1, &indexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    // VAOをOFFにしておく
    // Temporarily disable VAO
    glBindVertexArray(0);
}

void initializeGL(GLuint& programId, GLFWwindow* window) {
    // 深度テストの有効化
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // 背景色の設定 (黒)
    // Background color (black)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // VAOの初期化
    // Initialize VAO
    initVAO();

    // シェーダの用意
    // Prepare shader program
    programId = initShaders();
}

// ユーザ定義のOpenGL描画
// User-defined OpenGL drawing
void paintGL(GLuint programId, GLFWwindow* window, CA& ca) {
    // 背景色と深度値のクリア
    // Clear background color and depth values
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    TransMats *mats = (TransMats*)glfwGetWindowUserPointer(window);

    // 座標の変換. アークボール操作からモデル行列を決定する
    // Coordinate transformation. Model matrix is determined by arcball control
    glm::mat4 modelMat = mats->acTransMat * mats->acRotMat * mats->acScaleMat;
    glm::mat4 mvpMat = mats->projMat * mats->viewMat * modelMat;

    // シェーダの有効化
    // Enable shader program
    glUseProgram(programId);

    // Uniform変数の転送
    // Transfer uniform variables
    GLuint mvpMatLocId = glGetUniformLocation(programId, "u_mvpMat");
    glUniformMatrix4fv(mvpMatLocId, 1, GL_FALSE, glm::value_ptr(mvpMat));

    // VAOの有効化
    // Enable VAO
    glBindVertexArray(vaoId);

    auto&& field = ca.getField();

    // 三角形の描画
    // Draw triangles
    for(int i = 0; i < LENGTH; i++) {
        for(int j = 0; j < LENGTH; j++) {
            for(int k = 0; k < LENGTH; k++) {
                if(field[i][j][k]) {
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 
                            (void*)(((i * LENGTH * LENGTH) + (j * LENGTH) + k) * sizeof(GLuint) * 36));
                }
            }
        }
    }

    // VAOの無効化
    // Disable VAO
    glBindVertexArray(0);

    // シェーダの無効化
    // Disable shader program
    glUseProgram(0);
}

int main(int argc, char **argv) {
    int INIT_WIN_WIDTH = 500;
    int INIT_WIN_HEIGHT = 500;
    const char *WIN_TITLE = "OpenGL Course";

    GLuint programId;
    // OpenGLを初期化する
    // OpenGL initialization
    if (glfwInit() == GLFW_FALSE) {
        fprintf(stderr, "Initialization failed!\n");
        std::exit(EXIT_FAILURE);
    }

    // OpenGLのバージョン設定 (Macの場合には必ず必要)
    // Specify OpenGL version (mandatory for Mac)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Windowの作成
    // Create a window
    GLFWwindow *window = glfwCreateWindow(INIT_WIN_WIDTH, INIT_WIN_HEIGHT, WIN_TITLE,
                                          NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        fprintf(stderr, "Window creation failed!\n");
        std::exit(EXIT_FAILURE);
    }

    // OpenGLの描画対象にwindowを指定
    // Specify window as an OpenGL context
    glfwMakeContextCurrent(window);

    // OpenGL 3.x/4.xの関数をロードする (glfwMakeContextCurrentの後でないといけない)
    // Load OpenGL 3.x/4.x methods (must be loaded after "glfwMakeContextCurrent")
    const int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        fprintf(stderr, "Failed to load OpenGL 3.x/4.x libraries!\n");
        std::exit(EXIT_FAILURE);
    }

    // バージョンを出力する / Check OpenGL version
    printf("Load OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    initializeGL(programId, window);

    TransMats initTransMats = {
        glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::perspective(glm::radians(45.0f), (float)INIT_WIN_WIDTH / (float)INIT_WIN_HEIGHT, 0.1f, 1000.0f),
        glm::mat4(1.0),
        glm::mat4(1.0),
        glm::mat4(1.0),
        1.0f
    };

    glfwSetWindowUserPointer(window, &initTransMats);

    glfwSetWindowSizeCallback(window, resizeGL);
    glfwSetMouseButtonCallback(window, mouseEvent);
    glfwSetCursorPosCallback(window, motionEvent);
    glfwSetScrollCallback(window, wheelEvent);

    std::vector<int> alive_condition{3};
    CA ca = CA(LENGTH, alive_condition, 0.01, false, false);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        paintGL(programId, window, ca);
        ca.progressField();

        // 描画用バッファの切り替え
        // Swap drawing target buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}