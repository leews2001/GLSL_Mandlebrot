#include <iostream>
#include <stdlib.h>
#include <string>
#include <format>
#include <chrono>  

#include <glad/glad.h>
#include <gl/GL.h> 

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Input.h"
#include "shader.h"
#include "incl/fps.h"
#include "incl/deuxdouble.h"

// During movement (zoom, translate), for speedy interaction,
// rendering will be performed  at lower resolution of 
// (screen_width x screen_height) / SUB_RENDER_FACTOR.
constexpr int SUB_RENDER_FACTOR = 4;

// Function prototypes
void double_to_ds(double dval_, float& rout_val_, float& rout_err_);
void init_shaders();
void render_window_title(GLFWwindow* window, float fps_, int max_iter_, int precision_mode_);
void window_refresh_callback(GLFWwindow* window);
void win_resize_callback(GLFWwindow* window, int w, int h);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);

void update_camera( 
    bool& rout_b_update_cam,
    bool& rout_b_update_zoom,
    Input::Screen_Movement_t& r_mov_,
    Input::Screen_Camera_t& r_cam_);

void create_subres_texture( const int wd_, const int ht_, const int factor_, GLuint& rout_texture_);

//
Shader* gp_mdb_shader = nullptr;
Shader* gp_hud_shader = nullptr;
Shader* gp_upscale_shader = nullptr;

int scrn_wd{ 1080 };
int scrn_ht{ 1080 };

unsigned int VAO, VBO, EBO;


GLuint g_mdb_texture;

unsigned int quadVAO, quadVBO, quadEBO;

// Quad vertices for upscaling
float quadVertices[] = {
    // Positions
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f,
};

// Set up quad indices for upscaling
unsigned int quadIndices[] = {
    0, 1, 2,
    0, 2, 3
};


// Vertices for the crosshair (centered at the origin)
unsigned int crossVAO, crossVBO, crossEBO;

GLfloat crosshairVertices[] = {
    // Position      // Color
    -0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 0.7f, // Left point (red)
    0.5f, 0.0f, 0.0f,   1.0f, 0.0f, 0.7f, // Right point (red)
    0.0f, -0.5f, 0.0f,  1.0f, 0.0f, 0.7f, // Bottom point (red)
    0.0f, 0.5f, 0.0f,   1.0f, 0.0f, 0.7f  // Top point (red)
};




float vertices[] =
{
    //    x      y      z   
        -1.0f, -1.0f, -0.0f,
         1.0f,  1.0f, -0.0f,
        -1.0f,  1.0f, -0.0f,
         1.0f, -1.0f, -0.0f
};

unsigned int indices[] =
{
    //  2---,1
    //  | .' |
    //  0'---3
        0, 1, 2,
        0, 3, 1
};


using namespace std;

 

int main() 
{
    // Note: Long Double == Double for MSVC
    cout << "precision info:" << endl;
    cout << " float (min): " << std::numeric_limits<float>::min() << endl;
    cout << " float (digits10): " << std::numeric_limits<float>::digits10 << endl;
    cout << " double (min): " << std::numeric_limits<double>::min() << endl;
    cout << " double (digits10): " << std::numeric_limits<double>::digits10 << endl;
    cout << " long double (min): " << std::numeric_limits<long double>::min() << endl;
    cout<<" long double (digits10): " << std::numeric_limits<long double>::digits10 << endl;
    

    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(scrn_wd, scrn_ht, "Mandelbrot", NULL, NULL);

    if (window == nullptr) {

        cout << "Failed to create GLFW window!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }



    // Set the user pointer to pass the struct to the callback function
    Input g_input;
    glfwSetWindowUserPointer(window, &g_input);
     
    //-- set callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowRefreshCallback(window, window_refresh_callback);
    glfwSetWindowSizeCallback(window, win_resize_callback);
    glfwSetWindowAspectRatio(window, 1, 1);
  
     
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //
    {
        glGenVertexArrays(1, &crossVAO);
        glGenBuffers(1, &crossVBO);

        glBindVertexArray(crossVAO);

        glBindBuffer(GL_ARRAY_BUFFER, crossVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Color attribute (added)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        // Unbind VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    {
        // Set up vertex array object (VAO) and vertex buffer object (VBO) for quad
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);


    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    init_shaders();


   


    int _max_iter = 1000;
    gp_mdb_shader-> use_shader();

    gp_mdb_shader-> set_vec3("u_Color", glm::vec3(1.0f, 0.f, 0.f));
    gp_mdb_shader-> set_float("u_CameraZoom", 1.0 / g_input.m_scrn_cam.cameraZoom);
    gp_mdb_shader-> set_float("u_MaxIter", float(_max_iter));

    {
        float ds_val, ds_err;
        double_to_ds(g_input.m_scrn_cam.cameraTranslationX, ds_val, ds_err);
        gp_mdb_shader->set_vec2("u_ds_CameraPosX", glm::vec2(ds_val, ds_err));
        double_to_ds(g_input.m_scrn_cam.cameraTranslationY, ds_val, ds_err);
        gp_mdb_shader->set_vec2("u_ds_CameraPosY", glm::vec2(ds_val, ds_err));
    }


    // create sub-resolution texture for rendering
    create_subres_texture( scrn_wd, scrn_ht, SUB_RENDER_FACTOR, g_mdb_texture);


    // Set up mandelbrotFBO
    unsigned int mandelbrotFBO;
    glGenFramebuffers(1, &mandelbrotFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mandelbrotFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_mdb_texture, 0);

    // Reset framebuffer binding
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << endl;
    }
    else {
        cout << "Framebuffer OK!" << endl;
        // Unbind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //

    //--- 1: turn on v-sync, 0: off
    glfwSwapInterval(1);
    
    int _mode = 0;
    bool _b_idle = true;

    FPSCounter* _FPS = FPSCounter::getInstance();
    _FPS->start();

    //--- main render loop
    while (!glfwWindowShouldClose(window)) {

        bool b_update_cam{ false };
        bool b_update_zoom{ false };
        bool b_update_mode{ false };

        g_input.handle(_max_iter, b_update_mode);

        // toggle the rendering precision (double-single <-> double double) 
        if (b_update_mode) {
            _mode = (_mode + 1) % 2;
            b_update_mode = false;
        }

        double _fps = _FPS->update();
        render_window_title(window, _fps, _max_iter, _mode);

        update_camera( 
            b_update_cam,
            b_update_zoom,
            g_input.m_scrn_mov,
            g_input.m_scrn_cam);

        if (b_update_cam || b_update_zoom || b_update_mode) {
            // there is camera motion, view is being changed and we need to recalculate the Mandelbrot

            printf("[active] zoom:%.4e, fps:%.1f, iteration: %d, mode: %d\n",
                g_input.m_scrn_cam.cameraZoom, _fps, _max_iter, _mode);

            gp_mdb_shader-> use_shader();
            gp_mdb_shader-> set_float("u_MaxIter", float(_max_iter));
            gp_mdb_shader-> set_int("u_Mode", _mode);

            // Bind the framebuffer object (FBO) to render to
            glBindFramebuffer(GL_FRAMEBUFFER, mandelbrotFBO);

            glViewport(0, 0, scrn_wd / SUB_RENDER_FACTOR, scrn_ht / SUB_RENDER_FACTOR);


            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



            if (b_update_cam) {
                float ds_val, ds_err;
                
                double_to_ds(g_input.m_scrn_cam.cameraTranslationX, ds_val, ds_err);
                printf("[CAM] PX: %.20lf =  %.20f, %.20f\n", 
                    g_input.m_scrn_cam.cameraTranslationX, ds_val, ds_err);
                gp_mdb_shader->set_vec2("u_ds_CameraPosX", glm::vec2(ds_val, ds_err));

                double_to_ds(g_input.m_scrn_cam.cameraTranslationY, ds_val, ds_err);
                gp_mdb_shader->set_vec2("u_ds_CameraPosY", glm::vec2(ds_val, ds_err));
            }

            if (b_update_zoom) {

                double _zoom = 1.0 / g_input.m_scrn_cam.cameraZoom;

                gp_mdb_shader-> set_float("u_CameraZoom", static_cast<float>(_zoom));

            }

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            //////////////////



            ////////////////////////////////
            // Unbind the framebuffer to render to the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, scrn_wd, scrn_ht);
            {
                gp_upscale_shader->use_shader(); 
                glBindTexture(GL_TEXTURE_2D, g_mdb_texture); // Bind Mandelbrot texture to read from
                glBindVertexArray(quadVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            {
                // Render Crosshair
                gp_hud_shader->use_shader(); 
                glBindVertexArray(crossVAO);
                glLineWidth(3.0f);
                glDrawArrays(GL_LINES, 0, 4);
            }
            glBindVertexArray(0);

            _b_idle = false;

            glfwSwapBuffers(window);


        }
        else {

            if (!_b_idle) {
                //_max_iter = 1000;
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                gp_mdb_shader-> use_shader();


                //printf("[idle] iteration: 1000\n");
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                {
                    // Render Crosshair 
                    gp_hud_shader->use_shader();
                    glBindVertexArray(crossVAO);
                    glLineWidth(3.0f);
                    glDrawArrays(GL_LINES, 0, 4);

                }
                glBindVertexArray(0);
                _b_idle = true;
            }
            else {

                GLint _viewport[4];
                glGetIntegerv(GL_VIEWPORT, _viewport);

                glReadBuffer(GL_FRONT);
                glDrawBuffer(GL_BACK);

                glBlitFramebuffer(
                    _viewport[0], _viewport[1], _viewport[2], _viewport[3],
                    _viewport[0], _viewport[1], _viewport[2], _viewport[3],
                    GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);
            }

            glfwSwapBuffers(window);

        }

        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}


// Function definitions


void double_to_ds(double dval_, float& rout_val_, float& rout_err_)
{
    rout_val_ = (static_cast<float>(dval_));
    rout_err_ = (static_cast<float>(dval_ - rout_val_));

    return;
}

void init_shaders( )
{
    try {
        gp_mdb_shader = new Shader(
            "shaders/mandlebrot_shader.vs.glsl",
            "shaders/mandelbrot_shader_ds.fs.glsl");
   
        gp_hud_shader = new Shader(
            "shaders/hud_shader.vs.glsl",
            "shaders/hud_shader.fs.glsl");

        gp_upscale_shader = new Shader(
            "shaders/upscale_shader.vs.glsl",
            "shaders/upscale_shader.fs.glsl");
    }
    catch (const std::runtime_error& e) {
        // Handle the exception
        std::cerr << "Error: " << e.what() << endl;
        std::abort();
    }

    //Shader our_shader("shaders/mandlebrot_shader.vs.glsl", "shaders/mandlebrot_shader.fs.glsl");

    return;
}

void render_window_title(GLFWwindow* window, float fps_, int max_iter_, int precision_mode_ )
{
    //std::string msStr = std::to_string((timeDiff / counter) * 1000);
    std::string newTitle = 
        ( std::format("{:.1f}", fps_) 
            + "fps, max:"
            + std::format("{}", max_iter_) );

    if (precision_mode_ == 0) {
        newTitle += ", dS";
    } else if (precision_mode_ == 1) {
        newTitle += ", dD";
    }

    glfwSetWindowTitle(window, newTitle.c_str());

    return;

}

// Callback function for window refresh event
void window_refresh_callback( GLFWwindow* window)
{
    glClearColor(0.2f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gp_mdb_shader->use_shader(); 
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    {
        // Render Crosshair
        gp_hud_shader->use_shader(); 
        glBindVertexArray(crossVAO);
        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 0, 4);
    }
    glBindVertexArray(0);
    glfwSwapBuffers(window);

    // important, this waits until rendering result is actually visible, 
    // thus making resizing less ugly
    glFinish(); 
}

// Callback function for framebuffer size change event
void win_resize_callback(GLFWwindow* window, int w, int h)
{ 
    scrn_wd = w;
    scrn_ht = h;

    glViewport(0, 0, w, h);

    //-- resize the mandelbrot quarter-texture object too.
    glBindTexture(GL_TEXTURE_2D, g_mdb_texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        scrn_wd / SUB_RENDER_FACTOR,
        scrn_ht / SUB_RENDER_FACTOR,
        0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Future: Maintain 1:1 pixel-aspect ratio for non 1:1 window-aspect ratio
    //if (w > h) {
    //   glViewport(0, (h - w) / 2, w, w);
    //}
    //else {
    //    glViewport((w - h) / 2, 0, h, h);
    //}


    return;
}




// Callback function for key events
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods)
{
    // Handle key events using Input class
    Input* input = (Input*)glfwGetWindowUserPointer(win);
    input->check_keys(win, key, scancode, action, mods);
    return;
}

// Function to update camera parameters based on frame rate and user input
void update_camera(
    bool& rout_b_update_cam,
    bool& rout_b_update_zoom,
    Input::Screen_Movement_t& r_mov_,
    Input::Screen_Camera_t& r_cam_)
{
    if (r_mov_.reset_view) {
        r_cam_ = {}; // Use designated initializer to reset to default values
        rout_b_update_cam  = true;
        rout_b_update_zoom = true;
        return;
    }


    

    rout_b_update_zoom = (r_mov_.zoomIn || r_mov_.zoomOut);
    rout_b_update_cam = (r_mov_.moveDown || r_mov_.moveUp || r_mov_.moveLeft || r_mov_.moveRight);

    // camera translation
    double _pan = r_cam_.currentPanningSpeed;
    if (r_mov_.moveRight) { 

        r_cam_.cameraTranslationX += _pan;
   
    }
    else if (r_mov_.moveLeft) {
     
        printf("[Left] a) %.25lf\n", r_cam_.cameraTranslationX);

        long double A = r_cam_.cameraTranslationX;
        long double B = _pan;

        if (A < -1.0) {
            long double C = A + 1.0;
            printf("       *)  LD  C: %.25lf - B:%.25lf\n", C, B);
            
            C = C - B;
            printf("       *)  LD  C: %.25lf\n", C);
        }

        printf("       b)  LD  A: %.25lf - B:%.25lf\n", A, B);
        A = A - B;

 


        r_cam_.cameraTranslationX   = r_cam_.cameraTranslationX - _pan;
        printf("       c) A: %.25lf,  %.25lf\n",A, r_cam_.cameraTranslationX);
    }

    if (r_mov_.moveUp) {
 

        r_cam_.cameraTranslationY +=   _pan;
 

    }
    else if (r_mov_.moveDown) {
 
        r_cam_.cameraTranslationY -=  _pan;
 
    }

    // camera zoom
    if (r_mov_.zoomIn) {
        r_cam_.cameraZoom = r_cam_.cameraZoom * r_cam_.zoomSpeed; // Update the zoomAmount
        if (r_cam_.cameraZoom < 1) {
            r_cam_.cameraZoom = 1;
        }

        r_cam_.currentPanningSpeed = r_cam_.basePanningSpeed / r_cam_.cameraZoom; // Update camera panning speed

        printf("[pan] %.20lf\n", r_cam_.currentPanningSpeed);

    }
    else if (r_mov_.zoomOut) {
        r_cam_.cameraZoom = r_cam_.cameraZoom * (1 / r_cam_.zoomSpeed); // Update the zoomAmount

        if (r_cam_.cameraZoom < 1) {
            r_cam_.cameraZoom = 1;
        }

        r_cam_.currentPanningSpeed = r_cam_.basePanningSpeed / r_cam_.cameraZoom; // Update camera panning speed
        printf("[pan] %.20lf\n", r_cam_.currentPanningSpeed);
    }

    return;
}

/**
 * @brief Creates a texture with reduced resolution for sub-rendering purposes.
 *
 * @param[in] wd_  Width of window
 * @param[in] ht_  Height of window
 * @param[in] factor_ The factor by which to reduce the resolution of the texture.
 * @param[out] rout_texture_ Reference to store the generated texture ID.
 */
void create_subres_texture( const int wd_, const int ht_, const int factor_, GLuint &rout_texture_)
{
    glGenTextures(1, &rout_texture_);
    glBindTexture(GL_TEXTURE_2D, rout_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        wd_ / factor_, ht_ / factor_,
        0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // set filter and wrap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // texture to larger
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // texture to smaller
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return;
}