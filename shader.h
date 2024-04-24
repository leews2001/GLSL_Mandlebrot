#pragma once
#include <filesystem>  // For C++17 and above

#include <string>
 
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader
{
    unsigned int m_shader_id;

public:
   
    Shader(const std::filesystem::path& vertex_shader_path, 
            const std::filesystem::path& fragment_shader_path);

    ~Shader();
     
    void use_shader();


    // Setters for various shader uniforms...
    void set_int(const std::string& name, int value) const;

    void set_float(const std::string& name, float value) const;

    void set_double(const std::string& name, double value) const;
    void set_dvec2(const std::string& name, const glm::dvec2& value) const;

    void set_vec2(const std::string& name, const glm::vec2& value) const;
    void set_vec2(const std::string& name, float x, float y) const;

    void set_vec3(const std::string& name, const glm::vec3& value) const;
    void set_vec3(const std::string& name, float x, float y, float z) const;
    
    void set_vec4(const std::string& name, const glm::vec4& vec) const;
    void set_mat4(const std::string& name, const glm::mat4& mat) const;


private:
    void x_reset();
    void x_add_shader( const std::filesystem::path& shader_path, GLenum shader_type);
    auto x_read_shader_file(const std::filesystem::path& file_path)-> std::string;
    
};