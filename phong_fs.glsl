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

uniform sampler2D image;
uniform sampler2D bumpmap;
uniform vec3 diffuse;
uniform vec3 ambient;
uniform vec3 specular;
uniform float shininess;

in vec2 frag_mapcoord;
in vec3 frag_normal;
in vec3 frag_light_dir;
in vec3 frag_half_vector;

out vec3 frag_color;

vec3 get_normal() {
    return normalize(frag_normal);
}

vec3 compute_diffuse(vec3 normal, vec3 light_dir) {
    return diffuse * max(dot(normal, light_dir), 0);
}

vec3 compute_specular(vec3 normal, vec3 light_dir, vec3 half_vector) {
    if (dot(normal, light_dir) > 0)
        return specular * pow(max(dot(normal, half_vector), 0), shininess);
    else
        return vec3(0, 0, 0);
}

void main() {
    vec3 normal = get_normal();
    vec3 light_dir = normalize(frag_light_dir);
    vec3 half_vector = normalize(frag_half_vector);
    vec3 diffuse = compute_diffuse(normal, light_dir);
    vec3 specular = compute_specular(normal, light_dir, half_vector);
    vec3 color = texture(image, frag_mapcoord).xyz;
    frag_color = (diffuse + ambient) * color + specular;
}

