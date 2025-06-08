#include <grrlib.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <ogc/pad.h>
#include "BMfont3_png.h"

#define BLACK 0x000000FF
#define WHITE 0xFFFFFFFF
#define RED 0xFF0000FF
#define TEAL 0x008080FF

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

bool isAutoPlayMode = true;

bool isGamePaused;

typedef struct
{
    float x;
    float y;
    float w;
    float h;
    unsigned int color;
    bool isDestroyed;
    int brickPoints;
} Rectangle;

Rectangle player = {SCREEN_WIDTH / 2, SCREEN_HEIGHT - 16, 42, 16, WHITE};

Rectangle ball = {SCREEN_WIDTH / 2 - 16, SCREEN_HEIGHT / 2 - 16, 16, 16, WHITE};

const int playerSpeed = 6;

int ballVelocityX = 4;
int ballVelocityY = 4;

int playerScore;
int playerLives = 2;

std::vector<Rectangle> createBricks()
{
    std::vector<Rectangle> bricks;

    int brickPoints = 8;
    int positionX;
    int positionY = 50;

    for (int row = 0; row < 8; row++)
    {
        positionX = 0;

        for (int column = 0; column < 15; column++)
        {
            unsigned int color = RED;

            if (row % 2 == 0)
            {
                color = TEAL;
            }

            Rectangle actualBrick = {(float)positionX, (float)positionY, 41, 16, color, false, brickPoints};

            bricks.push_back(actualBrick);
            positionX += 43;
        }

        brickPoints--;
        positionY += 18;
    }

    return bricks;
}

std::vector<Rectangle> bricks = createBricks();

bool hasCollision(Rectangle bounds, Rectangle ball)
{
    return bounds.x < ball.x + ball.w && bounds.x + bounds.w > ball.x &&
           bounds.y < ball.y + ball.h && bounds.y + bounds.h > ball.y;
}

void update(u32 padDown, u32 padHeld)
{
    if (padDown & PAD_BUTTON_A)
    {
        isAutoPlayMode = !isAutoPlayMode;
    }

    if (isAutoPlayMode && ball.x < SCREEN_WIDTH - player.w)
    {
        player.x = ball.x;
    }

    if (padHeld & PAD_BUTTON_LEFT && player.x > 0)
    {
        player.x -= playerSpeed;
    }

    else if (padHeld & PAD_BUTTON_RIGHT && player.x < SCREEN_WIDTH - player.w)
    {
        player.x += playerSpeed;
    }

    if (ball.y > SCREEN_HEIGHT + ball.h)
    {
        ball.x = SCREEN_WIDTH / 2 - ball.w;
        ball.y = SCREEN_HEIGHT / 2 - ball.h;

        ballVelocityX *= -1;

        if (playerLives > 0)
        {
            playerLives--;
        }
    }

    if (ball.x < 0 || ball.x > SCREEN_WIDTH - ball.w)
    {
        ballVelocityX *= -1;
    }

    if (hasCollision(player, ball) || ball.y < 0)
    {
        ballVelocityY *= -1;
    }

    for (Rectangle &brick : bricks)
    {
        if (!brick.isDestroyed && hasCollision(brick, ball))
        {
            ballVelocityY *= -1;
            brick.isDestroyed = true;
            playerScore += brick.brickPoints;

            break;
        }
    }

    ball.x += ballVelocityX;
    ball.y += ballVelocityY;
}

int main(int argc, char **argv)
{
    GRRLIB_Init();
    PAD_Init();

    GRRLIB_texImg *tex_BMfont3 = GRRLIB_LoadTexture(BMfont3_png);
    GRRLIB_InitTileSet(tex_BMfont3, 32, 32, 32);

    while (true)
    {
        PAD_ScanPads();

        const u32 padDown = PAD_ButtonsDown(0);
        const u32 padHeld = PAD_ButtonsHeld(0);

        if (padDown & PAD_BUTTON_B)
        {
            break;
        }

        if (padDown & PAD_BUTTON_START)
        {
            isGamePaused = !isGamePaused;
        }

        if (!isGamePaused)
        {
            update(padDown, padHeld);
        }

        GRRLIB_FillScreen(BLACK);

        std::string scoreString = "SCORE: " + std::to_string(playerScore);

        GRRLIB_Printf(20, 0, tex_BMfont3, WHITE, 1, scoreString.c_str());

        std::string livesString = "LIVES: " + std::to_string(playerLives);

        GRRLIB_Printf(370, 0, tex_BMfont3, WHITE, 1, livesString.c_str());

        for (Rectangle brick : bricks)
        {
            if (!brick.isDestroyed)
            {
                GRRLIB_Rectangle(brick.x, brick.y, brick.w, brick.h, brick.color, 1);
            }
        }

        // the last value of the GRRLIB_Rectangle is for indicate if the rectangle should be draw filled or not.
        GRRLIB_Rectangle(ball.x, ball.y, ball.w, ball.h, ball.color, 1);
        GRRLIB_Rectangle(player.x, player.y, player.w, player.h, player.color, 1);

        if (isGamePaused)
        {
            GRRLIB_Printf(150, 50, tex_BMfont3, WHITE, 1, "GAME PAUSED");
        }

        GRRLIB_Render();
    }

    GRRLIB_FreeTexture(tex_BMfont3);
    GRRLIB_Exit();
    exit(0);
}
