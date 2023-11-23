#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define STATES 7
#define RADIUS 1
#define SCREEN_W 300
#define SCREEN_H 200
#define GRID_W 300
#define GRID_H 200
#define THRESHOLD 0.21f

#define SAVE_VIDEO

int compute(int* mnca, int** counts, int width, int height){
    const int radius = RADIUS;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            for(int s = 0; s < STATES; s++){
                counts[s][i * width + j] = 0;
            }
            for(int ii = -radius; ii <= radius; ii++){
                for(int jj = -radius; jj <= radius; jj++){
                    const int i_index = (i + ii + height) % height;
                    const int j_index = (j + jj + width) % width;
                    counts[mnca[i_index * width + j_index]][i * width + j]++;
                }
            }
        }
    }
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            float ps[STATES];
            for(int s = 0; s < STATES; s++){
                ps[s] = (float)counts[s][i * width + j] / ((2 * radius + 1) * (2 * radius + 1));
            }
            for(int s = 1; s <= STATES / 2; s++){
                if(ps[(mnca[i * width + j] + s) % STATES] > THRESHOLD){
                    mnca[i * width + j] = (mnca[i * width + j] + s) % STATES;
                    break;
                }
            }   
        }
    }
}
int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(SCREEN_W, SCREEN_H, "", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, SCREEN_W, SCREEN_H);

    GLuint texture;
    glGenTextures(1, &texture);

    const int width = GRID_W;
    const int height = GRID_H;

    unsigned char* tex_data = (unsigned char*)malloc(width * height * 3 * sizeof(unsigned char));
    int* mnca = (int*)malloc(width * height * sizeof(int));
    int* counts[STATES];
    int colors[STATES][3];
    for(int i = 0; i < STATES; i++){
        counts[i] = (int*)malloc(width * height * sizeof(int));
    }

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    srand(time(NULL));
    for(int i = 0; i < width; i++){
        for(int j = 0; j < height; j++){
            mnca[i * height + j] = rand() % STATES;
        }
    }
    for(int i = 0; i < STATES; i++){
        colors[i][0] = rand() % 256;
        colors[i][1] = rand() % 256;
        colors[i][2] = rand() % 256;
    }
    int frame = 0;
#ifdef SAVE_VIDEO
    //FILE *ffmpeg = popen("ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size 300x200 -framerate 30 -i - -c:v rawvideo -pix_fmt bgr24 -vf 'pad=ceil(iw/2)*2:ceil(ih/2)*2' output.avi", "w");
    FILE *ffmpeg = popen("ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size 300x200 -framerate 15 -i - -c:v libx264 -pix_fmt yuv420p -vf 'pad=ceil(iw/2)*2:ceil(ih/2)*2' output.mp4", "w");
#endif   
    while (!glfwWindowShouldClose(window)) {
        compute(mnca, counts, width, height);
        for (int i = 0; i < width * height; i++) {
            tex_data[3 * i + 0] = colors[mnca[i]][0];
            tex_data[3 * i + 1] = colors[mnca[i]][1];
            tex_data[3 * i + 2] = colors[mnca[i]][2];
        }
#ifdef SAVE_VIDEO
        if((frame % 5) == 0)
        {
            fwrite(tex_data, sizeof(uint8_t), width * height * 3, ffmpeg);
        }
#endif
        frame++;
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glDeleteTextures(1, &texture);
    glfwDestroyWindow(window);
    glfwTerminate();
    free(tex_data);
    free(mnca);
    for(int s = 0; s < STATES; s++){
        free(counts[s]);
    }
#ifdef SAVE_VIDEO
    pclose(ffmpeg);
#endif
    return 0;
}
