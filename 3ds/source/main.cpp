#include <citro2d.h>
#include <vector>

const int TOP_SCREEN_WIDTH = 400;
const int BOTTOM_SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

C3D_RenderTarget *topScreen = nullptr;
C3D_RenderTarget *bottomScreen = nullptr;

bool isGamePaused;
bool isAutoPlayMode = true;

C2D_TextBuf scoresBuffer;
C2D_TextBuf livesBuffer;

C2D_TextBuf gamePausedBuffer;
C2D_Text gamePauseTexts[1];

float textSize = 1.0f;

const u32 WHITE = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
const u32 BLACK = C2D_Color32(0x00, 0x00, 0x00, 0x00);
const u32 GREEN = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
const u32 RED = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
const u32 BLUE = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);

typedef struct
{
	float x;
	float y;
	float z;
	float w;
	float h;
	unsigned int color;
	bool isDestroyed;
	int brickPoints;
} Rectangle;

Rectangle player = {BOTTOM_SCREEN_WIDTH / 2, SCREEN_HEIGHT - 16, 0, 40, 8, WHITE};
Rectangle ball = {BOTTOM_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0, 10, 10, WHITE};
Rectangle touchBounds = {0, 0, 0, 8, 8, WHITE};

const int PLAYER_SPEED = 10;

int ballVelocityX = 5;
int ballVelocityY = 5;

int playerScore;
int playerLives = 2;

std::vector<Rectangle> createBricks()
{
	std::vector<Rectangle> bricks;
	bricks.reserve(70);

	int brickPoints = 10;
	int positionX;
	int positionY = 20;

	for (int row = 0; row < 10; row++)
	{
		positionX = 0;

		for (int column = 0; column < 7; column++)
		{
			unsigned int color = RED;

			if (row % 2 == 0)
			{
				color = BLUE;
			}

			Rectangle actualBrick = {(float)positionX, (float)positionY, 0, 41, 8, color, false, brickPoints};

			bricks.push_back(actualBrick);
			positionX += 43;
		}

		brickPoints--;
		positionY += 10;
	}

	return bricks;
}

std::vector<Rectangle> bricks = createBricks();

bool hasCollision(Rectangle &bounds, Rectangle &ball)
{
	return bounds.x < ball.x + ball.w && bounds.x + bounds.w > ball.x &&
		   bounds.y < ball.y + ball.h && bounds.y + bounds.h > ball.y;
}

void update()
{
	int keyHeld = hidKeysHeld();

	if (keyHeld & KEY_LEFT && player.x > 0)
	{
		player.x -= PLAYER_SPEED;
	}

	else if (keyHeld & KEY_RIGHT && player.x < BOTTOM_SCREEN_WIDTH - player.w)
	{
		player.x += PLAYER_SPEED;
	}

	if (ball.y > SCREEN_HEIGHT + ball.h)
	{
		ball.x = BOTTOM_SCREEN_WIDTH / 2;
		ball.y = SCREEN_HEIGHT / 2;

		ballVelocityX *= -1;

		if (playerLives > 0)
		{
			playerLives--;
		}
	}

	if (ball.x < 0 || ball.x > BOTTOM_SCREEN_WIDTH - ball.w)
	{
		ballVelocityX *= -1;
	}

	else if (hasCollision(player, ball) || ball.y < 0)
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

void renderTopScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(topScreen, BLACK);
	C2D_SceneBegin(topScreen);

	C2D_TextBufClear(scoresBuffer);
	C2D_TextBufClear(livesBuffer);

	char buffer[160];
	C2D_Text dynamicText;
	snprintf(buffer, sizeof(buffer), "Score: %d", playerScore);
	C2D_TextParse(&dynamicText, scoresBuffer, buffer);
	C2D_TextOptimize(&dynamicText);
	C2D_DrawText(&dynamicText, C2D_AlignCenter | C2D_WithColor, 100, 175, 0, textSize, textSize, WHITE);

	char buffer2[160];
	C2D_Text dynamicText2;
	snprintf(buffer2, sizeof(buffer2), "Lives: %d", playerLives);
	C2D_TextParse(&dynamicText2, livesBuffer, buffer2);
	C2D_TextOptimize(&dynamicText2);
	C2D_DrawText(&dynamicText2, C2D_AlignCenter | C2D_WithColor, 275, 175, 0, textSize, textSize, WHITE);

	if (isGamePaused)
	{
		C2D_DrawText(&gamePauseTexts[0], C2D_AtBaseline | C2D_WithColor, 110, 60, 0, textSize, textSize, WHITE);
	}

	C3D_FrameEnd(0);
}

void renderBottomScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(bottomScreen, BLACK);
	C2D_SceneBegin(bottomScreen);

	for (Rectangle &brick : bricks)
	{
		if (!brick.isDestroyed)
		{
			C2D_DrawRectSolid(brick.x, brick.y, brick.z, brick.w, brick.h, brick.color);
		}
	}

	C2D_DrawRectSolid(ball.x, ball.y, ball.z, ball.w, ball.h, ball.color);

	C2D_DrawRectSolid(player.x, player.y, player.z, player.w, player.h, player.color);

	C3D_FrameEnd(0);
}

int main(int argc, char *argv[])
{
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	gamePausedBuffer = C2D_TextBufNew(1024);
	scoresBuffer = C2D_TextBufNew(4096);
	livesBuffer = C2D_TextBufNew(4096);

	C2D_TextParse(&gamePauseTexts[0], gamePausedBuffer, "Game Paused");
	C2D_TextOptimize(&gamePauseTexts[0]);

	touchPosition touch;

	while (aptMainLoop())
	{
		hidScanInput();

		hidTouchRead(&touch);

		if (touch.px > 0 && touch.py > 0 && touch.px < BOTTOM_SCREEN_WIDTH - player.w)
		{
			player.x = touch.px;
		}

		touchBounds.x = touch.px;

		int keyDown = hidKeysDown();

		if (keyDown & KEY_START)
		{
			isGamePaused = !isGamePaused;
		}

		if (keyDown & KEY_A)
		{
			isAutoPlayMode = !isAutoPlayMode;
		}

		if (isAutoPlayMode && ball.x < BOTTOM_SCREEN_WIDTH - player.w)
		{
			player.x = ball.x;
		}

		if (!isGamePaused)
		{
			update();
		}

		renderTopScreen();
		renderBottomScreen();
	}

	C2D_TextBufDelete(scoresBuffer);
	C2D_TextBufDelete(livesBuffer);
	C2D_TextBufDelete(gamePausedBuffer);

	C2D_Fini();
	C3D_Fini();
	gfxExit();
}
