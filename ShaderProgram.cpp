/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Gabriel de Quadros Ligneul
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(const char* vs, const char* fs) :
    program_(glCreateProgram()) {
    CompileShader(GL_VERTEX_SHADER, vs); 
    CompileShader(GL_FRAGMENT_SHADER, fs);
    LinkShader();
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(program_);
}

unsigned int ShaderProgram::GetHandle() {
    return program_;
}

std::string ShaderProgram::ReadFile(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open())
        throw std::runtime_error("Unable to open file: " + path);
    std::string output;
    while (!input.eof()) {
        char line[1024];
        input.getline(line, 1024);
        output = output + line + "\n";
    }
    return output;
}

void ShaderProgram::Enable() {
    glUseProgram(program_);
}

void ShaderProgram::Disable() {
    glUseProgram(0);
}

void ShaderProgram::SetUniform(const std::string& name, int value) {
    GLuint location = glGetUniformLocation(program_, name.c_str());
    glUniform1i(location, value);
}

void ShaderProgram::SetUniform(const std::string& name, float value) {
    GLuint location = glGetUniformLocation(program_, name.c_str());
    glUniform1f(location, value);
}

void ShaderProgram::SetUniform(const std::string& name,
        const glm::vec3& value) {
    GLuint location = glGetUniformLocation(program_, name.c_str());
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::SetUniform(const std::string& name,
        const glm::vec4& value) {
    GLuint location = glGetUniformLocation(program_, name.c_str());
    glUniform4fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::SetUniform(const std::string& name,
        const glm::mat4& value) {
    GLuint location = glGetUniformLocation(program_, name.c_str());
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(value));
}

void ShaderProgram::SetAttribLocation(const char *name, unsigned int location) {
    glBindAttribLocation(program_, location, name);
}

void ShaderProgram::CompileShader(int shader_type, const std::string& path) {
    auto shader_str = ReadFile(path);
    auto shader_cstr = shader_str.c_str();
    auto shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_cstr, NULL);
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char log[length];
        glGetShaderInfoLog(shader, length, &length, log);
        glDeleteShader(shader);
        throw std::runtime_error(path + ": " + log);
    }
    glAttachShader(program_, shader);
}

void ShaderProgram::LinkShader() {
    glLinkProgram(program_);
    // TODO verify status
}

