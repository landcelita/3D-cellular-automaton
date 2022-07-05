#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include <glad/gl.h>
#include "shaders.h"


#define GLFW_DLL
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS        
#define GLM_ENABLE_EXPERIMENTAL  
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>


#include "common.h"


static int WIN_WIDTH = 500;
static int WIN_HEIGHT = 500;
static const char *WIN_TITLE = "OpenGL Course";

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
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f)
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

// マウスドラッグ中かどうか
// Flag to check mouse is dragged or not
bool isDragging = false;

// マウスのクリック位置
// Mouse click position
glm::ivec2 oldPos;
glm::ivec2 newPos;

// 操作の種類
// Type of control
enum ArcballMode {
    ARCBALL_MODE_NONE = 0x00,
    ARCBALL_MODE_TRANSLATE = 0x01,
    ARCBALL_MODE_ROTATE = 0x02,
    ARCBALL_MODE_SCALE = 0x04
};

// 座標変換のための変数
// Variables for coordinate transformation
int arcballMode = ARCBALL_MODE_NONE;
glm::mat4 viewMat, projMat;
glm::mat4 acRotMat, acTransMat, acScaleMat;
float acScale = 1.0f;

// VAOの初期化
// Initialize VAO
void initVAO() {
    // Vertex配列の作成
    // Create vertex array
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    int idx = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            Vertex v(positions[faces[i * 2 + 0][j]], colors[i]);
            vertices.push_back(v);
            indices.push_back(idx++);
        }

        for (int j = 0; j < 3; j++) {
            Vertex v(positions[faces[i * 2 + 1][j]], colors[i]);
            vertices.push_back(v);
            indices.push_back(idx++);
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

// ユーザ定義のOpenGLの初期化
// User-define OpenGL initialization
void initializeGL(GLuint& programId) {
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

    // カメラの姿勢を決定する変換行列の初期化
    // Initialize transformation matrices for camera pose
    projMat = glm::perspective(glm::radians(45.0f), (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);

    viewMat = glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f),   // 視点の位置 / Eye position
                          glm::vec3(0.0f, 0.0f, 0.0f),   // 見ている先 / Looking position
                          glm::vec3(0.0f, 1.0f, 0.0f));  // 視界の上方向 / Upward vector

    // アークボール操作のための変換行列を初期化
    // Initialize transformation matrices for arcball control
    acRotMat = glm::mat4(1.0);
    acTransMat = glm::mat4(1.0);
    acScaleMat = glm::mat4(1.0);
}

// ユーザ定義のOpenGL描画
// User-defined OpenGL drawing
void paintGL(GLuint programId) {
    // 背景色と深度値のクリア
    // Clear background color and depth values
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 座標の変換. アークボール操作からモデル行列を決定する
    // Coordinate transformation. Model matrix is determined by arcball control
    glm::mat4 modelMat = acTransMat * acRotMat * acScaleMat;
    glm::mat4 mvpMat = projMat * viewMat * modelMat;

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

    // 三角形の描画
    // Draw triangles
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // VAOの無効化
    // Disable VAO
    glBindVertexArray(0);

    // シェーダの無効化
    // Disable shader program
    glUseProgram(0);
}

// ウィンドウサイズ変更のコールバック関数
// Callback function for window resizing
void resizeGL(GLFWwindow *window, int width, int height) {
    // ユーザ管理のウィンドウサイズを変更
    // Update user-managed window size
    WIN_WIDTH = width;
    WIN_HEIGHT = height;

    // GLFW管理のウィンドウサイズを変更
    // Update GLFW-managed window size
    glfwSetWindowSize(window, WIN_WIDTH, WIN_HEIGHT);

    // 実際のウィンドウサイズ (ピクセル数) を取得
    // Get actual window size by pixels
    int renderBufferWidth, renderBufferHeight;
    glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);

    // ビューポート変換の更新
    // Update viewport transform
    glViewport(0, 0, renderBufferWidth, renderBufferHeight);

    // 東映変換行列の更新
    // Update projection matrix
    projMat = glm::perspective(glm::radians(45.0f), (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);
}

// マウスのクリックを処理するコールバック関数
// Callback for mouse click events
void mouseEvent(GLFWwindow *window, int button, int action, int mods) {
    // クリックしたボタンで処理を切り替える
    // Switch following operation depending on a clicked button
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        arcballMode = ARCBALL_MODE_ROTATE;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        arcballMode = ARCBALL_MODE_SCALE;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        arcballMode = ARCBALL_MODE_TRANSLATE;
    }

    // クリックされた位置を取得
    // Acquire a click position
    double px, py;
    glfwGetCursorPos(window, &px, &py);

    // マウスドラッグの状態を更新
    // Update state of mouse dragging
    if (action == GLFW_PRESS) {
        if (!isDragging) {
            isDragging = true;
            oldPos = glm::ivec2(px, py);
            newPos = glm::ivec2(px, py);
        }
    } else {
        isDragging = false;
        oldPos = glm::ivec2(0, 0);
        newPos = glm::ivec2(0, 0);
        arcballMode = ARCBALL_MODE_NONE;
    }
}

// スクリーン上の位置をアークボール球上の位置に変換する関数
// Convert screen-space coordinates to a position on the arcball sphere
glm::vec3 getVector(double x, double y) {
    // 円がスクリーンの長辺に内接していると仮定
    // Assume a circle contacts internally with longer edges
    const int shortSide = std::min(WIN_WIDTH, WIN_HEIGHT);
    glm::vec3 pt(2.0f * x / (float)shortSide - 1.0f, -2.0f * y / (float)shortSide + 1.0f, 0.0f);

    // z座標の計算
    // Calculate Z coordinate
    const double xySquared = pt.x * pt.x + pt.y * pt.y;
    if (xySquared <= 1.0) {
        // 単位円の内側ならz座標を計算
        // Calculate Z coordinate if a point is inside a unit circle
        pt.z = std::sqrt(1.0 - xySquared);
    } else {
        // 外側なら球の外枠上にあると考える
        // Suppose a point is on the circle line if the click position is outside the unit circle
        pt = glm::normalize(pt);
    }

    return pt;
}

// 回転成分の更新
// Update rotation matrix
void updateRotate() {
    // マウスクリック位置をアークボール球上の座標に変換
    // Convert click positions to those on the arcball sphere
    const glm::vec3 u = getVector(oldPos.x, oldPos.y);
    const glm::vec3 v = getVector(newPos.x, newPos.y);

    // カメラ座標における回転量 (=世界座標における回転量)
    // Amount of rotation in camera space (= that in world space)
    const double angle = std::acos(std::max(-1.0f, std::min(glm::dot(u, v), 1.0f)));

    // カメラ空間における回転軸
    // Rotation axis in camera space
    const glm::vec3 rotAxis = glm::cross(u, v);

    // カメラ座標の情報を世界座標に変換する行列
    // Transformation matrix from camera space to world space
    const glm::mat4 c2wMat = glm::inverse(viewMat);

    // 世界座標における回転軸
    // Rotation axis in world space
    const glm::vec3 rotAxisWorldSpace = glm::vec3(c2wMat * glm::vec4(rotAxis, 0.0f));

    // 回転行列の更新
    // Update rotation matrix
    acRotMat = glm::rotate((float)(4.0 * angle), rotAxisWorldSpace) * acRotMat;
}

// 平行移動量成分の更新
// Update translation matrix
void updateTranslate() {
    // NOTE:
    // この関数では物体が世界座標の原点付近にあるとして平行移動量を計算する
    // This function assumes the object locates near to the world-space origin and computes the amount of translation

    // 世界座標の原点のスクリーン座標を求める
    // Calculate screen-space coordinates of the world-space origin
    glm::vec4 originScreenSpace = (projMat * viewMat) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    originScreenSpace /= originScreenSpace.w;

    // スクリーン座標系におけるマウス移動の視点と終点の計算. これらの位置はスクリーン座標系のZ座標に依存することに注意する
    // Calculate the start and end points of mouse motion, which depend Z coordinate in screen space
    glm::vec4 newPosScreenSpace(2.0f * newPos.x / WIN_WIDTH - 1.0f, -2.0f * newPos.y / WIN_HEIGHT + 1.0f, originScreenSpace.z, 1.0f);
    glm::vec4 oldPosScreenSpace(2.0f * oldPos.x / WIN_WIDTH - 1.0f, -2.0f * oldPos.y / WIN_HEIGHT + 1.0f, originScreenSpace.z, 1.0f);

    // スクリーン座標の情報を世界座標座標に変換する行列 (= MVP行列の逆行列)
    // Transformation from screen space to world space (= inverse of MVP matrix)
    glm::mat4 invMvpMat = glm::inverse(projMat * viewMat);

    // スクリーン空間の座標を世界座標に変換
    // Transform screen-space positions to world-space positions
    glm::vec4 newPosObjSpace = invMvpMat * newPosScreenSpace;
    glm::vec4 oldPosObjSpace = invMvpMat * oldPosScreenSpace;
    newPosObjSpace /= newPosObjSpace.w;
    oldPosObjSpace /= oldPosObjSpace.w;

    // 世界座標系で移動量を求める
    // Calculate the amount of translation in world space
    const glm::vec3 transWorldSpace = glm::vec3(newPosObjSpace - oldPosObjSpace);

    // 行列に変換
    // Calculate translation matrix
    acTransMat = glm::translate(transWorldSpace) * acTransMat;
}

// 物体の拡大縮小率を更新
// Update object scale
void updateScale() {
    acScaleMat = glm::scale(glm::vec3(acScale, acScale, acScale));
}

// 変換行列の更新. マウス操作の内容に応じて更新対象を切り替える
// Update transformation matrices, depending on type of mouse interaction
void updateTransform() {
    switch (arcballMode) {
    case ARCBALL_MODE_ROTATE:
        updateRotate();
        break;

    case ARCBALL_MODE_TRANSLATE:
        updateTranslate();
        break;

    case ARCBALL_MODE_SCALE:
        acScale += (float)(oldPos.y - newPos.y) / WIN_HEIGHT;
        updateScale();
        break;
    }
}

// マウスの動きを処理するコールバック関数
// Callback for mouse move events
void motionEvent(GLFWwindow *window, double xpos, double ypos) {
    if (isDragging) {
        // マウスの現在位置を更新
        // Update current mouse position
        newPos = glm::ivec2(xpos, ypos);

        // マウスがあまり動いていない時は処理をしない
        // Update transform only when mouse moves sufficiently
        const double dx = newPos.x - oldPos.x;
        const double dy = newPos.y - oldPos.y;
        const double length = dx * dx + dy * dy;
        if (length < 2.0 * 2.0) {
            return;
        } else {
            updateTransform();
            oldPos = glm::ivec2(xpos, ypos);
        }
    }
}

// マウスホイールを処理するコールバック関数
// Callback for mouse wheel event
void wheelEvent(GLFWwindow *window, double xoffset, double yoffset) {
    acScale += yoffset / 10.0;
    updateScale();
}

int main(int argc, char **argv) {
    GLuint programId;

    // OpenGLを初期化する
    // OpenGL initialization
    if (glfwInit() == GLFW_FALSE) {
        fprintf(stderr, "Initialization failed!\n");
        return 1;
    }

    // OpenGLのバージョン設定 (Macの場合には必ず必要)
    // Specify OpenGL version (mandatory for Mac)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Windowの作成
    // Create a window
    GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE,
                                          NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        fprintf(stderr, "Window creation failed!\n");
        return 1;
    }

    // OpenGLの描画対象にwindowを指定
    // Specify window as an OpenGL context
    glfwMakeContextCurrent(window);

    // OpenGL 3.x/4.xの関数をロードする (glfwMakeContextCurrentの後でないといけない)
    // Load OpenGL 3.x/4.x methods (must be loaded after "glfwMakeContextCurrent")
    const int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        fprintf(stderr, "Failed to load OpenGL 3.x/4.x libraries!\n");
        return 1;
    }

    // バージョンを出力する / Check OpenGL version
    printf("Load OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    // ウィンドウのリサイズを扱う関数の登録
    // Register a callback function for window resizing
    glfwSetWindowSizeCallback(window, resizeGL);

    // マウスのイベントを処理する関数を登録
    // Register a callback function for mouse click events
    glfwSetMouseButtonCallback(window, mouseEvent);

    // マウスの動きを処理する関数を登録
    // Register a callback function for mouse move events
    glfwSetCursorPosCallback(window, motionEvent);

    // マウスホイールを処理する関数を登録
    // Register a callback function for mouse wheel
    glfwSetScrollCallback(window, wheelEvent);

    // ユーザ指定の初期化
    // User-specified initialization
    initializeGL(programId);

    // メインループ
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        // 描画 / Draw
        paintGL(programId);

        // 描画用バッファの切り替え
        // Swap drawing target buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 後処理 / Postprocess
    glfwDestroyWindow(window);
    glfwTerminate();
}