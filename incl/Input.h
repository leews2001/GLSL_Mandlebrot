#pragma once 

#include <array>
#include <unordered_map>

#include <iostream>
#include <functional>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

constexpr int N_KEYS = 349;
 

/**
 * @brief Input handler class for keyboard events.
 */
class Input {

private:
    // Define the key actions array as a private member of the class
    const std::array<std::pair<int, bool*>, 7> m_keyActions;
    const std::unordered_map<int, int> m_keyToRiterMap;
 
    std::array<bool, N_KEYS> m_pressed{}; // KEYS = 349 (last GLFW macro, GLFW_KEY_MENU = 348)

public:
    /**
     * @brief the movement state of the screen.
     */
    struct Screen_Movement_t {
        bool moveRight = false;
        bool moveLeft = false;
        bool moveUp = false;
        bool moveDown = false;
        bool zoomIn = false;
        bool zoomOut = false;
        bool reset_view = false;
    };


    /**
     * @brief the camera state of the screen.
     */
    struct Screen_Camera_t {
        double cameraTranslationX = -0.8;
        double cameraTranslationY = 0.;
        double basePanningSpeed = 0.05;// 0.025;
        double currentPanningSpeed = 0.025;
        double cameraZoom = 1e0;
        double zoomSpeed = 1.05f;
    };

    Screen_Movement_t m_scrn_mov;
    Screen_Camera_t   m_scrn_cam;

public:
    // Constructor
    Input();

    // Function declarations

    /**
     * @brief Checks the state of keys based on GLFW events.
     *
     * This function is responsible for updating the pressed state of keys
     * based on GLFW keyboard events.
     */
    void check_keys(GLFWwindow* win, int key, int scancode, int action, int mods);

    /**
     * @brief Handles the input events.
     *
     * This function handles the input events and updates the movement state
     * and r_iter_ value accordingly.
     */
    void handle(int& r_iter_, bool& b_mode_);

    /**
     * @brief Checks the state of keys locally.
     *
     * This function checks the state of keys directly using GLFW functions
     * and updates the movement state accordingly.
     */
    void local_check_keys(GLFWwindow* window);
};