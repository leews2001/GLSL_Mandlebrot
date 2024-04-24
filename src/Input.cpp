#include "Input.h"

// Constructor
Input::Input():
    m_keyActions{ {
        { GLFW_KEY_R, &m_scrn_mov.reset_view },
        { GLFW_KEY_D, &m_scrn_mov.moveRight },
        { GLFW_KEY_A, &m_scrn_mov.moveLeft },
        { GLFW_KEY_W, &m_scrn_mov.moveUp },
        { GLFW_KEY_S, &m_scrn_mov.moveDown },
        { GLFW_KEY_Q, &m_scrn_mov.zoomIn },
        { GLFW_KEY_E, &m_scrn_mov.zoomOut }, 
    } },
    m_keyToRiterMap({
        { GLFW_KEY_0, 100 },
        { GLFW_KEY_1, 1000 },
        { GLFW_KEY_2, 2000 },
        { GLFW_KEY_3, 4000 },
        { GLFW_KEY_4, 8000 },
        { GLFW_KEY_5, 12000 }
        })
{
   // constructor implementation....
}

/**
 * @brief Check the state of keys.
 *
 * This function checks the state of keys and performs actions accordingly.
 *
 * @param win The GLFWwindow pointer.
 * @param key The key being checked.
 * @param scancode The scancode of the key.
 * @param action The action being performed (e.g., GLFW_PRESS).
 * @param mods The modifier keys being pressed (e.g., GLFW_MOD_SHIFT).
 */
void Input::check_keys(
    GLFWwindow* win, 
    int key, 
    int scancode, 
    int action, 
    int mods) 
{
    //printf("\n[check keys] key: %d, action: %d\n", key, action);

    // Don't accept unknown keys
    if (key == GLFW_KEY_UNKNOWN) {
        return;
    }

     // Handle special keys
    switch (key) {

        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS)
                glfwSetWindowShouldClose(win, true);
            break;
        case GLFW_KEY_V:
            if (action == GLFW_PRESS) {
                if (mods == GLFW_MOD_SHIFT) {
                    printf("[Vsync] ON\n");
                    glfwSwapInterval(1);
                }
                else {
                    printf("[Vsync] OFF\n");
                    glfwSwapInterval(0);
                }
            }
            break;
    }

    // Set pressed state based on action
    m_pressed[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);

    return;
}

/**
 * @brief Handle key presses,( Things to do every frame?)
 *
 * This function handles key presses and sets movement flags accordingly.
 *
 * @param r_iter_ The reference to the iteration count.
 * @param b_mode_ The reference to the mode flag.
 */
void Input::handle(int& r_iter_, bool& b_mode_, bool& b_xhair_) 
{
        
    // Iterate over each key code and its corresponding action
    for (const auto& [key, action] : m_keyActions) {
        *action = m_pressed[key];
    }


    // Reset mode flag if 'M' is pressed
    b_mode_ = m_pressed[GLFW_KEY_M];
    m_pressed[GLFW_KEY_M] = false; // Reset pressed state for 'M' key

    if (m_pressed[GLFW_KEY_X]) {
        b_xhair_ = !b_xhair_;
        m_pressed[GLFW_KEY_X] = false;
    }
    // Check if any key corresponding to r_iter_ is pressed
    for (const auto& pair : m_keyToRiterMap) {
        if (m_pressed[pair.first]) {
            r_iter_ = pair.second;
            break;  // Exit loop after setting r_iter_
        }
    }

    //--- nullify conflicting movement
    if (m_scrn_mov.zoomIn && m_scrn_mov.zoomOut) {
        m_scrn_mov.zoomIn = false;
        m_scrn_mov.zoomOut = false;
    }

    if (m_scrn_mov.moveRight && m_scrn_mov.moveLeft) {
        m_scrn_mov.moveRight = false;
        m_scrn_mov.moveLeft = false;
    }

    if (m_scrn_mov.moveUp && m_scrn_mov.moveDown) {
        m_scrn_mov.moveUp = false;
        m_scrn_mov.moveDown = false;
    }
    return;
}

/**
 * @brief Local check for key presses.
 *
 * This function checks for key presses locally and sets movement flags accordingly.
 *
 * @param window The GLFWwindow pointer.
 */
void Input::local_check_keys(GLFWwindow* window)
{
    // Iterate over each key code and its corresponding action
    for (const auto& [key, action] : m_keyActions) {
        // Check state of key
        int state = glfwGetKey(window, key);
        // Update the corresponding value
        *action = (state == GLFW_PRESS || state == GLFW_REPEAT);
    }

    return;
}