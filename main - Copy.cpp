#include <iostream>
#include <stdlib.h>
#include <string>
#include <chrono> 
#include <thread>

#include <glad/glad.h>
#include <gl/GL.h> 

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "Input.h"
#include "shader.h"
#include "incl/fps.h"

 
int screen_width{ 1080 };
int screen_height{ 1080 };
 
unsigned int VAO, VBO, EBO;
int g_ourshader_id;
int g_hudshader_id;
int g_upscaleshader_id;

unsigned int mandelbrotTexture;

 
/////////////////////
unsigned int quadVAO, quadVBO, quadEBO;

// quad vertices for upscaling
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

///////////////
// Vertices for the cross (centered at the origin)

unsigned int crossVAO, crossVBO, crossEBO;

GLfloat crossVertices[] = {
    // Position      // Color
    -0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 0.7f, // Left point (red)
    0.5f, 0.0f, 0.0f,   1.0f, 0.0f, 0.7f, // Right point (red)
    0.0f, -0.5f, 0.0f,  1.0f, 0.0f, 0.7f, // Bottom point (red)
    0.0f, 0.5f, 0.0f,   1.0f, 0.0f, 0.7f  // Top point (red)
};

// Indices for the cross lines
//GLuint crossIndices[] = {
//    0, 1, // Horizontal line
//    2, 3  // Vertical line
//};

///////////////


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



void window_refresh_callback(GLFWwindow* window)
{
    glClearColor(0.2f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_ourshader_id);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    {
        // Render Crosshair
        glUseProgram(g_hudshader_id);
        glBindVertexArray(crossVAO);
        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 0, 4);
       
    }
    glBindVertexArray(0);
    glfwSwapBuffers(window);
    glFinish(); // important, this waits until rendering result is actually visible, thus making resizing less ugly
}
 

void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    printf("[resize] %d x %d\n", w, h);
    screen_width = w;
    screen_height = h;
 
    glViewport(0, 0, w, h);
 
    //-- resize the mandelbrot quarter-texture object too.
    glBindTexture(GL_TEXTURE_2D, mandelbrotTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, 
        screen_width / 4, 
        screen_height / 4, 
        0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    //-- future: maintaining 1:1 pixel-aspect ratio
    //   for non 1:1 window-aspect ratio.
    //if (w > h) {
    //   glViewport(0, (h - w) / 2, w, w);
    //  
    //}
    //else {
    //    glViewport((w - h) / 2, 0, h, h);
    //}
     
 
    return;
}


void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    Input* input = (Input*)glfwGetWindowUserPointer(win);
    input->check_keys(win, key, scancode, action, mods);
    return;
}


void update_camera(
    double fps_,
    bool &rout_b_update_cam, 
    bool &rout_b_update_zoom, 
    Screen_Movement_t &r_mov_,
    Screen_Camera_t &r_cam_)
{
    if (r_mov_.reset_view) {
        r_cam_ = {}; // Use designated initializer to reset to default values
        rout_b_update_cam = true;
        rout_b_update_zoom = true;
        return;
    }

    if (fps_ < .5) fps_ = 1.0;

    double _pan = r_cam_.currentPanningSpeed;// *(60.0 / fps_);
   

    rout_b_update_zoom = (r_mov_.zoomIn || r_mov_.zoomOut);
    rout_b_update_cam = (r_mov_.moveDown || r_mov_.moveUp || r_mov_.moveLeft || r_mov_.moveRight);

    // camera translation
    if (r_mov_.moveRight) {
        printf("[PAN] RIGHT, ");
         
        r_cam_.cameraTranslation += glm::vec2( _pan, 0.0f);
        printf("  [PAN] %lf, trans: %lf, %lf\n", _pan, r_cam_.cameraTranslation.x, r_cam_.cameraTranslation.y);
    } else if (r_mov_.moveLeft) {
        printf("[PAN] LEFT, ");
     
        r_cam_.cameraTranslation += glm::vec2(-_pan, 0.0f);
        printf("  [PAN] %lf, trans: %lf, %lf\n", _pan, r_cam_.cameraTranslation.x, r_cam_.cameraTranslation.y);
    }

    if (r_mov_.moveUp) {
        printf("[PAN] UP, ");
        
        r_cam_.cameraTranslation += glm::vec2(0.0f, _pan);
        printf("  [PAN] %lf, trans: %lf, %lf\n", _pan, r_cam_.cameraTranslation.x, r_cam_.cameraTranslation.y);

    } else if (r_mov_.moveDown) {
        printf("[PAN] DOWN, "); 
        r_cam_.cameraTranslation += glm::vec2(0.0f, -_pan);
        printf("  [PAN] %lf, trans: %lf, %lf\n", _pan, r_cam_.cameraTranslation.x, r_cam_.cameraTranslation.y);
    }


   

    // camera zoom
    if (r_mov_.zoomIn) {
        r_cam_.cameraZoom = r_cam_.cameraZoom * r_cam_.zoomSpeed; // Update the zoomAmount
        if (r_cam_.cameraZoom < 1) {
            r_cam_.cameraZoom = 1;
        }

        r_cam_.currentPanningSpeed = r_cam_.basePanningSpeed / r_cam_.cameraZoom; // Update camera panning speed
        if (r_cam_.currentPanningSpeed < 1E-7) {
            r_cam_.currentPanningSpeed = 1E-7f;
        } 
    } else if (r_mov_.zoomOut) {
        r_cam_.cameraZoom = r_cam_.cameraZoom * (1 / r_cam_.zoomSpeed); // Update the zoomAmount

        if (r_cam_.cameraZoom < 1) {
            r_cam_.cameraZoom = 1;
        }

        r_cam_.currentPanningSpeed = r_cam_.basePanningSpeed / r_cam_.cameraZoom; // Update camera panning speed
        if (r_cam_.currentPanningSpeed < 1E-7) {
            r_cam_.currentPanningSpeed = 1E-7f;
        } 
    }

    return;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Mandelbrot", NULL, NULL);

    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }



    // Set the user pointer to pass the struct to the callback function
    Input g_input;
    glfwSetWindowUserPointer(window, &g_input);


    //-- set callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowRefreshCallback(window, window_refresh_callback);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowAspectRatio(window, 1, 1);
    //-- init 
  //  framebuffer_size_callback( window, screen_width, screen_height);
     
 

    //unsigned int VAO, VBO, EBO;
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(crossVertices), crossVertices, GL_STATIC_DRAW);

 
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
 
    Shader our_shader("shaders/mandlebrot_shader.vs.glsl", "shaders/mandelbrot_shader_ds.fs.glsl");
    g_ourshader_id = our_shader.program_ID;

    Shader hud_shader("shaders/hud_shader.vs.glsl", "shaders/hud_shader.fs.glsl");
    g_hudshader_id = hud_shader.program_ID;

    Shader upscale_shader("shaders/upscale_shader.vs.glsl", "shaders/upscale_shader.fs.glsl");
    g_upscaleshader_id = upscale_shader.program_ID;
 
    //Shader our_shader("shaders/mandlebrot_shader.vs.glsl", "shaders/mandlebrot_shader.fs.glsl");
 

    our_shader.use_shader();
   
    float _max_iter = 1000;

 
    {
        our_shader.set_vec3("u_Color", glm::vec3(1.0f, 0.f, 0.f));
        our_shader.set_float("u_CameraZoom", 1.0/g_scr_cam.cameraZoom);
         our_shader.set_float("u_MaxIter", _max_iter);
        our_shader.set_vec2("u_ds_CameraPosX", glm::vec2(g_scr_cam.cameraTranslation.x, 0.));
        our_shader.set_vec2("u_ds_CameraPosY", glm::vec2(g_scr_cam.cameraTranslation.y, 0.));
    }
 
 

   // glEnable(GL_DEPTH_TEST);
     
  
    //--- 1: turn on v-sync, 0: off
    glfwSwapInterval(1);

    int _mode =  0;
    bool vsync = false;
    
    bool _b_idle = true;


    FPSCounter* _FPS = FPSCounter::getInstance();
    _FPS->start();

    //--------------------------------
    // Load Mandelbrot texture

 

    
    {
        glGenTextures(1, &mandelbrotTexture);
        glBindTexture(GL_TEXTURE_2D, mandelbrotTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width / 4, screen_height / 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        // set filter and wrap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // texture to larger
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // texture to smaller
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Set up mandelbrotFBO
    unsigned int mandelbrotFBO;
    glGenFramebuffers(1, &mandelbrotFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mandelbrotFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mandelbrotTexture, 0);
        
    // Reset framebuffer binding
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /////////////    ;
    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }
    else {
        std::cout << "Framebuffer OK!" << std::endl;
        // Unbind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    while (!glfwWindowShouldClose(window)) {

    

        bool b_update_cam{ false };
        bool b_update_zoom{ false };
        bool b_update_mode{ false };

       g_input.handle( _max_iter , b_update_mode);

        if (b_update_mode) {
            _mode = (_mode + 1) % 2;
        }

        bool  b_have_update = false;
        double _fps = _FPS-> update(window, b_have_update); 

 /*       if (b_have_update) {
            g_input.local_check_keys(window);
        }*/

        update_camera(
            _fps,
            b_update_cam, 
            b_update_zoom, 
            g_input.m_screen_mov, 
            g_scr_cam);
       
        if (b_update_cam || b_update_zoom || b_update_mode) { 
       
            our_shader.use_shader();
           
            printf("[active] zoom:%.4e, fps:%.1f, iteration: %.1f, mode: %d\n", 
                g_scr_cam.cameraZoom, _fps, _max_iter, _mode);


            our_shader.set_float("u_MaxIter", _max_iter);

            if (b_update_mode) {
                our_shader.set_int("u_Mode", _mode);
                b_update_mode = false;
            }
            // Bind the framebuffer object (FBO) to render to
            glBindFramebuffer(GL_FRAMEBUFFER, mandelbrotFBO); 

            glViewport(0, 0, screen_width / 4, screen_height / 4);


            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 


            if (b_update_cam) {
                our_shader.set_vec2("u_ds_CameraPosX", glm::vec2(g_scr_cam.cameraTranslation.x, 0.));
                our_shader.set_vec2("u_ds_CameraPosY", glm::vec2(g_scr_cam.cameraTranslation.y, 0.));
            }

            if (b_update_zoom) {
 
                double _zoom = 1.0 / g_scr_cam.cameraZoom;

                our_shader.set_float("u_CameraZoom", static_cast<float>(_zoom));
 
            }

             glBindVertexArray(VAO);
             glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

             //////////////////

            
             
             ////////////////////////////////
             // Unbind the framebuffer to render to the default framebuffer
             glBindFramebuffer(GL_FRAMEBUFFER, 0);
             glViewport(0, 0, screen_width, screen_height);
             {
                 //glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind default framebuffer
                 //glViewport(0, 0, screen_width, screen_height);
                 glUseProgram( g_upscaleshader_id);
                 glBindTexture(GL_TEXTURE_2D, mandelbrotTexture); // Bind Mandelbrot texture
                 glBindVertexArray(quadVAO);
                 glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                 glBindTexture(GL_TEXTURE_2D, 0);
             }

             {
                 // Render Crosshair
                 glUseProgram(g_hudshader_id);
                 glBindVertexArray(crossVAO);
                 glLineWidth(3.0f);
                 glDrawArrays(GL_LINES, 0, 4);
             }
             glBindVertexArray(0);
           
          

            glfwSwapBuffers(window);
            _b_idle = false;

        }
        else { 

            if (!_b_idle) {
                //_max_iter = 1000;
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                our_shader.use_shader();
                

                //printf("[idle] iteration: 1000\n");
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                {
                    // Render Crosshair
                    glUseProgram(g_hudshader_id);
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