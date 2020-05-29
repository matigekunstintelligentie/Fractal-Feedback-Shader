//g++ fractalRender.cpp -o fractalRender glad.c -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lGLU -lGL `pkg-config --cflags --libs opencv`
// changed from <"glad/glad.h"> also in glad.cpp download from https://glad.dav1d.de/
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include "./shaders_s.h"
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

float zoom = 400.;
float zoom_factor = 1.1;
float x_pan = 0.0;
float y_pan = 0.0;
float x_offset = -1920.f/2.0;
float y_offset = -1080.f/2.0;
bool record = false;




FILE *ffmpeg = popen("/usr/bin/ffmpeg -vcodec rawvideo -f rawvideo -y -pix_fmt rgba -s 1920x1080 -i pipe:0 -vcodec h264 -r 24 out.mp4", "w");
int* buffer = new int[SCR_WIDTH*SCR_HEIGHT];

int main()
{



    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fractal", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("3.3.shader.vs", "3.3.shader.fs"); // you can name your shader files however you like



    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions         // colors
        1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,1.0f,  1.0f,  // top right
        1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  0.0f, // bottom left
        -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  1.0f
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };




    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);

    VideoCapture cap(0);
    Mat image;
    bool success = cap.read(image);





    // image = imread("images/image.png");



    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0,
                GL_BGR, GL_UNSIGNED_BYTE, image.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);




    ourShader.use();
    int juliaModeLocation = glGetUniformLocation(ourShader.ID, "julia_mode");
    glUniform1i(juliaModeLocation, 1);

    int zoomLocation = glGetUniformLocation(ourShader.ID, "zoom");
    glUniform1f(zoomLocation, zoom);

    int x_panLocation = glGetUniformLocation(ourShader.ID, "x_pan");
    glUniform1f(x_panLocation, x_pan);

    int y_panLocation = glGetUniformLocation(ourShader.ID, "y_pan");
    glUniform1f(y_panLocation, y_pan);

    int x_offsetLocation = glGetUniformLocation(ourShader.ID, "x_offset");
    glUniform1f(x_panLocation, x_offset);

    int y_offsetLocation = glGetUniformLocation(ourShader.ID, "y_offset");
    glUniform1f(y_offsetLocation, y_offset);

    int juliaLocation = glGetUniformLocation(ourShader.ID, "julia");


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        success = cap.read(image);
        flip(image, image, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);


        glBindTexture(GL_TEXTURE_2D, texture);


        // render the triangle
        ourShader.use();

        float timeValue = glfwGetTime();
        float sine = cos(timeValue) / 2.0f + 0.5f + sin(timeValue)*0.5;
        float cosine = sin(timeValue) / 2.0f + 0.5f + 0.6*sin(timeValue + 0.1);
        glUniform2f(juliaLocation, -0.8 + sine*0.5, 0.156 + cosine*0.5);

        glUniform1f(zoomLocation, zoom);
        glUniform1f(x_panLocation, x_pan);
        glUniform1f(y_panLocation, y_pan);
        glUniform1f(x_offsetLocation, x_offset);
        glUniform1f(y_offsetLocation, y_offset);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        if(record){
            glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
            fwrite(buffer, sizeof(int)*SCR_WIDTH*SCR_HEIGHT, 1, ffmpeg);
        }

        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    pclose(ffmpeg);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
        record = true;
    }

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        record = false;
    }

    if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){
        zoom *= zoom_factor;
        x_pan = zoom_factor * x_pan;
        y_pan = zoom_factor * y_pan;
        std::cout << zoom << std::endl;
    }

    if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS){
        zoom /= zoom_factor;
        x_pan = x_pan/zoom_factor;
        y_pan = y_pan/zoom_factor;
    }

    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        x_pan += 10;
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        x_pan -= 10;
    }

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        y_pan += 10;
    }

    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        y_pan -= 10;
    }


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
