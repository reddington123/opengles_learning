/* Copyright (c) 2013-2017, ARM Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* [Includes] */
#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <GLES3/gl3.h>

#define LOG_TAG "libNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
/* [Includes] */

// Vertex and fragment shaders (simple shaders to render a circle)
const char *vertexShaderSource = R"(#version 320 es
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    out vec3 ourColor;
    out vec3 ourPosition;
    uniform float horizontalOffset;
     void main()
    {
        ourColor =  aColor;
        // Normal triangle downside up
        gl_Position = vec4(aPos,1.0);
        // triangle is upside down
        // gl_Position = vec4(aPos.x ,-aPos.y,aPos.z,1.0);
        ourPosition = aPos;
    }
)";

const char *fragmentShaderSource = R"(#version 320 es
    mediump out vec4 FragColor;
    mediump in vec3 ourColor;
    mediump in vec3 ourPosition;
    void main()
    {
        FragColor = vec4(ourPosition,1.0f);
        // FragColor = vec4(ourColor,1.0f);
    }

)";

GLuint loadShader(GLenum shaderType, const char* shaderSource)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader != 0)
    {
        glShaderSource(shader, 1, &shaderSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (compiled != GL_TRUE)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

            if (infoLen > 0)
            {
                char * logBuffer = (char*) malloc(infoLen);

                if (logBuffer != NULL)
                {
                    glGetShaderInfoLog(shader, infoLen, NULL, logBuffer);
                    LOGE("Could not Compile Shader %d:\n%s\n", shaderType, logBuffer);
                    free(logBuffer);
                    logBuffer = NULL;
                }

                glDeleteShader(shader);
                shader = 0;
            }
        }
    }

    return shader;
}

GLuint createProgram(const char* vertexSource, const char * fragmentSource)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0)
    {
        return 0;
    }

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0)
    {
        return 0;
    }

    GLuint program = glCreateProgram();

    if (program != 0)
    {
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if(linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength > 0)
            {
                char* logBuffer = (char*) malloc(bufLength);

                if (logBuffer != NULL)
                {
                    glGetProgramInfoLog(program, bufLength, NULL, logBuffer);
                    LOGE("Could not link program:\n%s\n", logBuffer);
                    free(logBuffer);
                    logBuffer = NULL;
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

float vertices[] = {
        // positions         // colors
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
        0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
};

GLuint VAO, VBO,simpleTriangleProgram;
GLint hoffset;
void setup_objects()
{
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6* sizeof(float),(void *)NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6* sizeof(float),(void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

bool setupGraphics(int width, int height) {
    simpleTriangleProgram = createProgram(vertexShaderSource, fragmentShaderSource);
    if (simpleTriangleProgram == 0) {
        LOGE("Could not create program");
        return false;
    }
    glUseProgram(simpleTriangleProgram);
    hoffset = glGetUniformLocation(simpleTriangleProgram,"horizontalOffset");
    if(hoffset != -1) {
        glUniform1f(hoffset,0.5f);
    }
    else {
        LOGE("Uniform horizontalOffset not found.");
    }
    glViewport(0, 0, width, height);
    setup_objects();
    return true;
}


void renderFrame(int width, int height) {
    glUseProgram(simpleTriangleProgram);
    // glClearColor(0.0f,1.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES,0,3);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_opengles_1learning_NativeLibrary_init(JNIEnv *env, jclass clazz, jint width,
                                                       jint height) {
    setupGraphics(1080, 2340);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_opengles_1learning_NativeLibrary_step(JNIEnv *env, jclass clazz) {
    renderFrame(1080, 2340);
}