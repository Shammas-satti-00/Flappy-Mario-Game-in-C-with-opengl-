#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <GL/glut.h>
#include <string>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Bird properties
float birdY = 0.0f;
float birdVelocity = 0.0f;
const float gravity = -0.001f;
const float flapStrength = 0.03f;
bool gameStarted = false;
bool gameOver = false;

// Pipe properties
const int numPipes = 4;
float pipeX[numPipes];
float pipeGap[numPipes];
float pipeSpeed = 0.01f;
const float pipeGapHeight = 0.6f;
const float pipeWidth = 0.1f;
const float pipeSpacing = 0.6f;

// Square properties
const int numSquares = 2;
float squareX[numSquares];
float squareY[numSquares];
const float squareSize = 0.1f;

// Score
int score = 0;

// Difficulty level
enum Difficulty { EASY, HARD };
Difficulty difficulty = EASY;
bool difficultyChosen = false;

// Background and bird textures
GLuint birdTexture;

void renderBitmapString(float x, float y, void* font, const char* string) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void chooseDifficulty() {
    difficultyChosen = false;
}

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (!image) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        exit(1);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, (channels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(image);
    return texture;
}

void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set the clear color to white
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    srand(static_cast<unsigned int>(time(0)));

    // Initialize pipes
    for (int i = 0; i < numPipes; ++i) {
        pipeX[i] = 1.0f + i * pipeSpacing;
        pipeGap[i] = (rand() % 100 / 100.0f) * 0.6f - 0.3f;
    }

    // Initialize squares
    for (int i = 0; i < numSquares; ++i) {
        squareX[i] = 1.0f + i * pipeSpacing / 2.0f;
        squareY[i] = (rand() % 100 / 100.0f) * 1.8f - 0.9f;
    }

    // Load bird texture
    birdTexture = loadTexture("mario.png");
}

void drawBackground() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White background
    glClear(GL_COLOR_BUFFER_BIT);
}

void drawBird() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, birdTexture);
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the texture

    float birdWidth = 0.3f; // Adjust bird width
    float birdHeight = 0.3f; // Adjust bird height

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-0.5f, birdY - birdHeight / 2);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(-0.4f, birdY - birdHeight / 2);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(-0.4f, birdY + birdHeight / 2);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-0.5f, birdY + birdHeight / 2);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawPipe(float x, float gapY) {
    glColor3f(0.0f, 0.8f, 0.0f);

    glBegin(GL_QUADS);

    // Bottom pipe
    glVertex2f(x, -1.0f);
    glVertex2f(x + pipeWidth, -1.0f);
    glVertex2f(x + pipeWidth, gapY - pipeGapHeight / 2);
    glVertex2f(x, gapY - pipeGapHeight / 2);

    // Top pipe
    glVertex2f(x, gapY + pipeGapHeight / 2);
    glVertex2f(x + pipeWidth, gapY + pipeGapHeight / 2);
    glVertex2f(x + pipeWidth, 1.0f);
    glVertex2f(x, 1.0f);

    glEnd();
}

void drawSquare(float x, float y) {
    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    glVertex2f(x - squareSize / 2, y - squareSize / 2);
    glVertex2f(x + squareSize / 2, y - squareSize / 2);
    glVertex2f(x + squareSize / 2, y + squareSize / 2);
    glVertex2f(x - squareSize / 2, y + squareSize / 2);
    glEnd();
}

void display() {
    drawBackground();

    drawBird();

    for (int i = 0; i < numPipes; ++i) {
        drawPipe(pipeX[i], pipeGap[i]);
    }

    for (int i = 0; i < numSquares; ++i) {
        drawSquare(squareX[i], squareY[i]);
    }

    // Display the difficulty level menu if the difficulty is not chosen
    if (!difficultyChosen) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red color
        renderBitmapString(-0.2f, 0.3f, GLUT_BITMAP_HELVETICA_18, "Choose Difficulty:");
        renderBitmapString(-0.2f, 0.2f, GLUT_BITMAP_HELVETICA_18, "E - Easy");
        renderBitmapString(-0.2f, 0.1f, GLUT_BITMAP_HELVETICA_18, "H - Hard");
    }

    // Display the score if the game has started
    if (gameStarted) {
        glColor3f(0.0f, 0.0f, 0.0f); // Black color for score
        std::string scoreText = "Score: " + std::to_string(score);
        renderBitmapString(0.7f, 0.9f, GLUT_BITMAP_HELVETICA_18, scoreText.c_str());
    }

    // Display game over message if the game is over
    if (gameOver) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red color
        renderBitmapString(-0.1f, 0.0f, GLUT_BITMAP_HELVETICA_18, "Game Over");
    }

    glutSwapBuffers();
}

void update(int value) {
    if (gameStarted && !gameOver) {
        birdVelocity += gravity;
        birdY += birdVelocity;

        for (int i = 0; i < numPipes; ++i) {
            pipeX[i] -= pipeSpeed;
            if (pipeX[i] < -1.0f - pipeWidth) {
                pipeX[i] = 1.0f;
                pipeGap[i] = (rand() % 100 / 100.0f) * 0.6f - 0.3f;
                ++score;
                PlaySound(TEXT("coin.wav"), NULL, SND_ASYNC);
            }

            // Check for collision with pipes
            if (pipeX[i] > -0.5f - pipeWidth && pipeX[i] < -0.5f + pipeWidth) {
                if (birdY < pipeGap[i] - pipeGapHeight / 2 || birdY > pipeGap[i] + pipeGapHeight / 2) {
                    gameOver = true;
                }
            }
        }

        for (int i = 0; i < numSquares; ++i) {
            squareX[i] -= pipeSpeed;
            if (squareX[i] < -1.0f - squareSize / 2) {
                squareX[i] = 1.0f;
                squareY[i] = (rand() % 100 / 100.0f) * 1.8f - 0.9f;
            }

            // Check for collision with squares
            if (squareX[i] > -0.5f - squareSize / 2 && squareX[i] < -0.5f + squareSize / 2 &&
                birdY > squareY[i] - squareSize / 2 && birdY < squareY[i] + squareSize / 2) {
                ++score;
                squareX[i] = 1.0f;
                squareY[i] = (rand() % 100 / 100.0f) * 1.8f - 0.9f;
                PlaySound(TEXT("coin.wav"), NULL, SND_ASYNC);
            }
        }

        // Check for collision with ground or ceiling
        if (birdY < -1.0f || birdY > 1.0f) {
            gameOver = true;
        }

        glutPostRedisplay();
    }

    glutTimerFunc(16, update, 0); // Schedule next update
}

void handleKeyboard(unsigned char key, int x, int y) {
    if (!difficultyChosen) {
        if (key == 'e' || key == 'E') {
            difficulty = EASY;
            difficultyChosen = true;
            gameStarted = false;
        }
        else if (key == 'h' || key == 'H') {
            difficulty = HARD;
            pipeSpeed = 0.02f;
            difficultyChosen = true;
            gameStarted = false;
        }
    }
    else {
        if (key == ' ' && !gameOver) {
            if (!gameStarted) {
                gameStarted = true;
                birdVelocity = flapStrength;
            }
            else {
                birdVelocity = flapStrength;
            }
            PlaySound(TEXT("flap.wav"), NULL, SND_ASYNC);
        }

        if (key == 'r' || key == 'R') {
            birdY = 0.0f;
            birdVelocity = 0.0f;
            gameStarted = false;
            gameOver = false;
            score = 0;
            pipeSpeed = (difficulty == HARD) ? 0.02f : 0.01f;

            for (int i = 0; i < numPipes; ++i) {
                pipeX[i] = 1.0f + i * pipeSpacing;
                pipeGap[i] = (rand() % 100 / 100.0f) * 0.6f - 0.3f;
            }

            for (int i = 0; i < numSquares; ++i) {
                squareX[i] = 1.0f + i * pipeSpacing / 2.0f;
                squareY[i] = (rand() % 100 / 100.0f) * 1.8f - 0.9f;
            }
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Flappy Bird");

    init();

    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0); // Start the update loop
    glutKeyboardFunc(handleKeyboard);

    glutMainLoop();
    return 0;
}
