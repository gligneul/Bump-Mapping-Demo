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

#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <lodepng.h>

#include "Manipulator.h"
#include "ShaderProgram.h"
#include "VertexArray.h"
#include "Texture2D.h"

// Types
struct Image {
    std::vector<unsigned char> data;
    unsigned int w, h;

    // Obtains a pixel
    glm::vec4 get(int i, int j) {
        return {
            data[0 + i * 4 + j * w * 4] / 255.0f,
            data[1 + i * 4 + j * w * 4] / 255.0f,
            data[2 + i * 4 + j * w * 4] / 255.0f,
            data[3 + i * 4 + j * w * 4] / 255.0f,
        };
    }

    // Defines a pixel
    void set(int i, int j, glm::vec4 v) {
        data[0 + i * 4 + j * w * 4] = v.x * 255.0f;
        data[1 + i * 4 + j * w * 4] = v.y * 255.0f;
        data[2 + i * 4 + j * w * 4] = v.z * 255.0f;
        data[3 + i * 4 + j * w * 4] = v.w * 255.0f;
    }
};

// Constants
static const int kN = 64;
static const int kM = 64;
static const int kNumIndices = 6 * (kN - 1) * (kM - 1);

// Window size
static int window_w = 1920;
static int window_h = 1080;

// Helpers
static Manipulator manipulator;
static ShaderProgram shader;
static VertexArray ball;
static Texture2D image;
static Texture2D bumpmap;

// Matrices
static glm::mat4 model;
static glm::mat4 view;
static glm::mat4 projection;
static glm::mat4 mvp;
static glm::mat4 model_inv;

// Configurable variables
static char vertex_shader[128] = {0};
static char fragment_shader[128] = {0};
static char image_path[128] = {0};
static char hmap_path[128] = {0};
static glm::vec3 eye;
static glm::vec3 center;
static glm::vec3 up;
static glm::vec4 light;
static glm::vec3 diffuse;
static glm::vec3 ambient;
static glm::vec3 specular;
static float shininess = 0.0f;

// Verifies the condition, if it fails, shows the error message and
// exits the program
#define Assert(condition, format, ...) { \
    if (!condition) { \
        auto finalformat = std::string("Error at function %s: ") \
                + format + "\n"; \
        fprintf(stderr, finalformat.c_str(), __func__, __VA_ARGS__); \
        exit(1); \
    } \
}

// Loads the shader
static void CreateShader() {
    try {
        shader.LoadVertexShader(vertex_shader);
        shader.LoadFragmentShader(fragment_shader);
        shader.LinkShader();
    } catch (std::exception& e) {
        Assert(false, "%s", e.what());
    }
}

// Creates the sphere coordinates based on a grid
static void CreateSphereCoordinates(std::vector< unsigned int >& indices,
                                    std::vector< float >& vertices,
                                    std::vector< float >& normals,
                                    std::vector< float >& tangents,
                                    std::vector< float >& binormals,
                                    std::vector< float >& textcoords) {
    for (int i = 0; i < kN; ++i) {
        for (int j = 0; j < kM; ++j) {
            const float pi = M_PI;
            float s = (float)i / (kN - 1);
            float t = (float)j / (kM - 1);
            float theta = (1 - t) * pi;
            float phi = (1 - s) * 2.0f * pi;
            float x = sin(theta) * cos(phi);
            float y = cos(theta);
            float z = sin(theta) * sin(phi);
            auto tangent = glm::normalize(glm::vec3(
                -2 * pi * sin(2 * pi * s) * sin(pi * t),
                0,
                -2 * pi * cos(2 * pi * s) * sin(pi * t)
            ));
            auto binormal = glm::normalize(glm::vec3(
                pi * cos(2 * pi * s) * cos(pi * t),
                pi * sin(pi * t),
                -pi * sin(2 * pi * s) * cos(pi * t)
            ));
            auto normal = glm::normalize(glm::vec3(x, y, z));
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
            tangents.push_back(tangent.x);
            tangents.push_back(tangent.y);
            tangents.push_back(tangent.z);
            binormals.push_back(binormal.x);
            binormals.push_back(binormal.y);
            binormals.push_back(binormal.z);
            textcoords.push_back(s);
            textcoords.push_back(t);
        }
    }

    auto Index = [&](int i, int j) {
        return j + i * kM;
    };

    for (int i = 0; i < kN - 1; ++i) {
        for (int j = 0; j < kM - 1; ++j) {
            indices.push_back(Index(i, j));
            indices.push_back(Index(i, j + 1));
            indices.push_back(Index(i + 1, j));
            indices.push_back(Index(i + 1, j));
            indices.push_back(Index(i, j + 1));
            indices.push_back(Index(i + 1, j + 1));
        }
    }
} 

// Creates the sphere
static void CreateSphere() {
    std::vector< unsigned int > indices;
    std::vector< float > vertices, normals, tangents, binormals, textcoords;
    CreateSphereCoordinates(indices, vertices, normals, tangents, binormals,
            textcoords);

    ball.Init();
    ball.SetElementArray(indices.data(), indices.size());
    ball.AddArray(0, vertices.data(), vertices.size(), 3);
    ball.AddArray(1, normals.data(), normals.size(), 3);
    ball.AddArray(2, tangents.data(), tangents.size(), 3);
    ball.AddArray(3, binormals.data(), binormals.size(), 3);
    ball.AddArray(4, textcoords.data(), textcoords.size(), 2);
}

// Updates the variables that depend on the model, view and projection
static void UpdateMatrices() {
    model = manipulator.GetMatrix(glm::normalize(center - eye));
    view = glm::lookAt(eye, center, up);
    auto ratio = (float)window_w / (float)window_h;
    projection = glm::perspective(glm::radians(60.0f), ratio, 0.1f, 10.0f);
    mvp = projection * view * model;
    model_inv = glm::inverse(model);
}

// Load a png image to memory
static Image LoadPNG(const char* path) {
    Image image;
    unsigned int error = lodepng::decode(image.data, image.w, image.h, path);
    Assert(!error, "lodepng error: %s", lodepng_error_text(error));
    for (unsigned int i = 0; i < image.w; ++i) {
        for (unsigned int j = 0; j < image.h / 2; ++j) {
            auto top = image.get(i, j);
            auto bottomidx = image.h - 1 - j;
            auto bottom = image.get(i, bottomidx);
            image.set(i, j, bottom);
            image.set(i, bottomidx, top);
        }
    }
    return image;
}

// Create a bump map based on elevation map
static Image CreateBumpMap(Image elevation) {
    Image bump;
    bump.w = elevation.w - 1;
    bump.h = elevation.h - 1;
    bump.data.resize(4 * bump.w * bump.h);
    for (unsigned int i = 0; i < bump.w; ++i) {
        for (unsigned int j = 0; j < bump.h; ++j) {
            float e00 = elevation.get(i, j).x;
            float e01 = elevation.get(i, j + 1).x;
            float e10 = elevation.get(i + 1, j).x;
            glm::vec3 nv(0, 1, e01 - e00);
            glm::vec3 nh(1, 0, e10 - e00);
            auto n = normalize(glm::cross(nh, nv));
            auto c = n / 2.0f + 0.5f;
            bump.set(i, j, glm::vec4(c, 1));
        }
    }
    return bump;
}

// Loads the texture
static void CreateTextures() {
    auto LoadTexture = [](Texture2D* texture, Image image) {
        texture->LoadTexture(image.data.data(), image.w, image.h);
    };
    LoadTexture(&image, LoadPNG(image_path));
    LoadTexture(&bumpmap, CreateBumpMap(LoadPNG(hmap_path)));
}

// Loads the global opengl configuration
static void LoadGlobalConfiguration() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
}

// Loads the shader and creates the sphere
static void CreateScene() {
    LoadGlobalConfiguration();
    CreateShader();
    CreateSphere();
    CreateTextures();
    UpdateMatrices();
}

// Loads the shader's uniform variables
static void LoadShaderVariables() {
    shader.SetUniform("mvp", mvp);
    shader.SetUniform("model_inv", model_inv);
    shader.SetUniform("light_pos", light);
    shader.SetUniform("eye_pos", eye);
    shader.SetUniform("diffuse", diffuse);
    shader.SetUniform("ambient", ambient);
    shader.SetUniform("specular", specular);
    shader.SetUniform("shininess", shininess);
    shader.SetTexture2D("image", 0, image.GetId());
    shader.SetTexture2D("bumpmap", 1, bumpmap.GetId());
}

// Display callback, renders the sphere
static void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.Enable();
    LoadShaderVariables();
    ball.DrawElements(GL_TRIANGLES, kNumIndices, GL_UNSIGNED_INT);
    shader.Disable();
}

// Resize callback
static void Reshape(GLFWwindow *window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width == window_w && height == window_h)
        return;

    window_w = width;
    window_h = height;
    glViewport(0, 0, width, height);
    UpdateMatrices();
}

// Keyboard callback
static void Keyboard(GLFWwindow* window, int key, int scancode, int action,
                     int mods) {
    if (action != GLFW_PRESS)
        return;

    switch (key) {
        case GLFW_KEY_Q:
            exit(0);
            break;
        default:
            break;
    }
}

// Mouse Callback
static void Mouse(GLFWwindow *window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    auto manip_button = (button == GLFW_MOUSE_BUTTON_LEFT) ? 0 :
                        (button == GLFW_MOUSE_BUTTON_RIGHT) ? 1 : -1;
    auto pressed = action == GLFW_PRESS;
    manipulator.MouseClick(manip_button, pressed, (int)x, (int)y);
    UpdateMatrices();
}

// Motion callback
static void Motion(GLFWwindow *window, double x, double y) {
    manipulator.MouseMotion((int)x, (int)y);
    UpdateMatrices();
}

// Loads the configuration file
static void LoadConfiguration(int argc, char *argv[]) {
    const char *config_file = argc > 1 ? argv[1] : "config.txt";
    FILE *f = fopen(config_file, "r");
    Assert(f, "cannot open configuration file %s", config_file);

    fscanf(f, "vertex_shader: %127s\n", vertex_shader);
    fscanf(f, "fragment_shader: %127s\n", fragment_shader);
    fscanf(f, "image_path: %127s\n", image_path);
    fscanf(f, "hmap_path: %127s\n", hmap_path);
    fscanf(f, "eye: %f, %f, %f\n", &eye.x, &eye.y, &eye.z);
    fscanf(f, "center: %f, %f, %f\n", &center.x, &center.y, &center.z);
    fscanf(f, "up: %f, %f, %f\n", &up.x, &up.y, &up.z);
    fscanf(f, "light: %f, %f, %f, %f\n", &light.x, &light.y, &light.z, &light.w);
    fscanf(f, "diffuse: %f, %f, %f\n", &diffuse.x, &diffuse.y, &diffuse.z);
    fscanf(f, "ambient: %f, %f, %f\n", &ambient.x, &ambient.y, &ambient.z);
    fscanf(f, "specular: %f, %f, %f\n", &specular.x, &specular.y, &specular.z);
    fscanf(f, "shininess: %f\n", &shininess);

    fclose(f);
}

// Initialization
int main(int argc, char *argv[]) {
    // Init glfw
    Assert(glfwInit(), "glfw init failed", 0);
    auto window = glfwCreateWindow(window_w, window_h, "OpenGL4 Application",
            nullptr, nullptr);
    Assert(window, "glfw window couldn't be created", 0);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, Keyboard);
    glfwSetMouseButtonCallback(window, Mouse);
    glfwSetCursorPosCallback(window, Motion);

    // Init glew
    auto glew_error = glewInit();
    Assert(!glew_error, "GLEW error: %s", glewGetErrorString(glew_error));

    // Init application
    LoadConfiguration(argc, argv);
    CreateScene();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        Reshape(window);
        Display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    };

    glfwTerminate();
    return 0;
}

