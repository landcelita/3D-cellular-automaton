#include "event_handler.h"
#include <glad/gl.h>
#define GLFW_DLL
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

// マウスドラッグ中かどうか
// Flag to check mouse is dragged or not
bool isDragging = false;

// 操作の種類
// Type of control
enum ArcballMode {
    ARCBALL_MODE_NONE = 0x00,
    ARCBALL_MODE_TRANSLATE = 0x01,
    ARCBALL_MODE_ROTATE = 0x02,
    ARCBALL_MODE_SCALE = 0x04
};
static int arcballMode = ARCBALL_MODE_NONE;

// マウスのクリック位置
// Mouse click position
static glm::ivec2 oldPos;
static glm::ivec2 newPos;

// ウィンドウサイズ変更のコールバック関数
// Callback function for window resizing
void resizeGL(GLFWwindow *window, int width, int height) {
    TransMats *mats = (TransMats*)glfwGetWindowUserPointer(window);

    glfwSetWindowSize(window, width, height);

    int renderBufferWidth, renderBufferHeight;
    glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);

    glViewport(0, 0, renderBufferWidth, renderBufferHeight);

    mats->projMat = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
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

    double px, py;
    glfwGetCursorPos(window, &px, &py);

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
glm::vec3 getVector(double x, double y, int winWidth, int winHeight) {
    // 円がスクリーンの長辺に内接していると仮定
    const int shortSide = std::min(winWidth, winHeight);
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
void updateRotate(TransMats* mats, int winWidth, int winHeight) {
    // マウスクリック位置をアークボール球上の座標に変換
    // Convert click positions to those on the arcball sphere
    const glm::vec3 u = getVector(oldPos.x, oldPos.y, winWidth, winHeight);
    const glm::vec3 v = getVector(newPos.x, newPos.y, winWidth, winHeight);

    // カメラ座標における回転量 (=世界座標における回転量)
    // Amount of rotation in camera space (= that in world space)
    const double angle = std::acos(std::max(-1.0f, std::min(glm::dot(u, v), 1.0f)));

    // カメラ空間における回転軸
    // Rotation axis in camera space
    const glm::vec3 rotAxis = glm::cross(u, v);

    // カメラ座標の情報を世界座標に変換する行列
    // Transformation matrix from camera space to world space
    const glm::mat4 c2wMat = glm::inverse(mats->viewMat);

    // 世界座標における回転軸
    // Rotation axis in world space
    const glm::vec3 rotAxisWorldSpace = glm::vec3(c2wMat * glm::vec4(rotAxis, 0.0f));

    // 回転行列の更新
    // Update rotation matrix
    mats->acRotMat = glm::rotate((float)(4.0 * angle), rotAxisWorldSpace) * mats->acRotMat;
}

// 平行移動量成分の更新
// Update translation matrix
void updateTranslate(TransMats* mats, int winWidth, int winHeight) {
    // NOTE:
    // この関数では物体が世界座標の原点付近にあるとして平行移動量を計算する
    // This function assumes the object locates near to the world-space origin and computes the amount of translation

    // 世界座標の原点のスクリーン座標を求める
    // Calculate screen-space coordinates of the world-space origin
    glm::vec4 originScreenSpace = (mats->projMat * mats->viewMat) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    originScreenSpace /= originScreenSpace.w;

    // スクリーン座標系におけるマウス移動の視点と終点の計算. これらの位置はスクリーン座標系のZ座標に依存することに注意する
    // Calculate the start and end points of mouse motion, which depend Z coordinate in screen space
    glm::vec4 newPosScreenSpace(2.0f * newPos.x / winWidth - 1.0f, -2.0f * newPos.y / winHeight + 1.0f, originScreenSpace.z, 1.0f);
    glm::vec4 oldPosScreenSpace(2.0f * oldPos.x / winWidth - 1.0f, -2.0f * oldPos.y / winHeight + 1.0f, originScreenSpace.z, 1.0f);

    // スクリーン座標の情報を世界座標座標に変換する行列 (= MVP行列の逆行列)
    // Transformation from screen space to world space (= inverse of MVP matrix)
    glm::mat4 invMvpMat = glm::inverse(mats->projMat * mats->viewMat);

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
    mats->acTransMat = glm::translate(transWorldSpace) * mats->acTransMat;
}

// 物体の拡大縮小率を更新
// Update object scale
void updateScale(TransMats* mats) {
    mats->acScaleMat = glm::scale(glm::vec3(mats->acScale, mats->acScale, mats->acScale));
}

void updateTransform(TransMats* mats, int winWidth, int winHeight) {
    switch (arcballMode) {
    case ARCBALL_MODE_ROTATE:
        updateRotate(mats, winWidth, winHeight);
        break;

    case ARCBALL_MODE_TRANSLATE:
        updateTranslate(mats, winWidth, winHeight);
        break;

    case ARCBALL_MODE_SCALE:
        mats->acScale += (float)(oldPos.y - newPos.y) / winHeight;
        updateScale(mats);
        break;
    }
}

// マウスの動きを処理するコールバック関数
// Callback for mouse move events
void motionEvent(GLFWwindow *window, double xpos, double ypos) {
    int winWidth, winHeight;
    TransMats *mats = (TransMats*)glfwGetWindowUserPointer(window);

    glfwGetWindowSize(window, &winWidth, &winHeight);
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
            updateTransform(mats, winWidth, winHeight);
            oldPos = glm::ivec2(xpos, ypos);
        }
    }
}

// マウスホイールを処理するコールバック関数
// Callback for mouse wheel event
void wheelEvent(GLFWwindow *window, double xoffset, double yoffset) {
    TransMats *mats = (TransMats*)glfwGetWindowUserPointer(window);
    mats->acScale += yoffset / 10.0;
    updateScale(mats);
}