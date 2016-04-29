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

#version 450

uniform mat4 mvp;
uniform mat4 model_inv;
uniform vec4 light_pos;
uniform vec3 eye_pos;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 binormal;
layout(location = 4) in vec2 textcoord;

out vec2 frag_mapcoord;
out vec3 frag_normal;
out vec3 frag_light_dir;
out vec3 frag_half_vector;

vec3 get_dir_ms(vec4 v_gs) {
    vec4 v_ms4 = model_inv * v_gs;
    vec3 v_ms = v_ms4.xyz / v_ms4.w;
    return normalize(v_ms - position);
}

void main() {
    vec3 eye_dir = get_dir_ms(vec4(eye_pos, 1));
    vec3 light_dir = get_dir_ms(light_pos);
    vec3 half_vector = normalize(light_dir + eye_dir);

    gl_Position = mvp * vec4(position, 1);
    frag_mapcoord = textcoord;
    frag_normal = normal;
    frag_light_dir = light_dir;
    frag_half_vector = half_vector;
}

