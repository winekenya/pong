#include "windows.h"
#include "math.h"

const float PI = 3.1416;

typedef struct sprite {
	float x, y, width, height, rad, dx, dy, speed;
	bool status;
	HBITMAP hBitmap;
};

const int dimX = 20;
const int dimY = 6;
sprite blocks[dimY][dimX];
sprite racket;
sprite ball;

struct game {
	int score = 0, balls = 4;
	bool action = false;
} game;

struct window {
	HWND hWnd;
	HDC device_context, context;
	int width, height;
} window;

HBITMAP hBack;

///////////////////////////////////

void InitGame()
{
	//перенес все изображения в папку Resourses
	ball.hBitmap = (HBITMAP)LoadImageA(NULL, "../Resourses/ball10.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); 
	racket.hBitmap = (HBITMAP)LoadImageA(NULL, "../Resourses/racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	hBack = (HBITMAP)LoadImageA(NULL, "../Resourses/back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	racket.width = 300;
	racket.height = 50;
	racket.speed = 30;
	racket.x = window.width / 2.;
	racket.y = window.height - racket.height;

	ball.dy = (rand() % 65 + 35) / 100.;
	ball.dx = (1 - ball.dy);
	ball.speed = 111;
	ball.rad = 10; //уменьшил радиус шарика
	ball.x = racket.x;
	ball.y = racket.y - ball.rad;

	for (int y = 0; y < dimY; y++)
	{
		int x;
		if (y % 2 == 0) x = 1;
		else x = 2;

		for (; x < dimX - 1; x+=2)
		{
			sprite& block = blocks[y][x];

			block.hBitmap = (HBITMAP)LoadImageA(NULL, "../Resourses/block.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

			block.width = (window.width) / dimX;
			block.height = window.height / 4 / dimY;

			block.x = x * (block.width);
			block.y = y * (block.height) + window.height / 4;

			block.status = true;
		}
	}

	game.score = 0;
	game.balls = 4;
}

void ShowScore()
{
	SetTextColor(window.context, RGB(255, 255, 255));
	SetBkColor(window.context, RGB(0, 0, 0));
	SetBkMode(window.context, TRANSPARENT);
	auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
	auto hTmp = (HFONT)SelectObject(window.context, hFont);

	char txt[32];
	_itoa_s(game.score, txt, 10);
	TextOutA(window.context, 10, 10, "Score", 5);
	TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

	_itoa_s(game.balls, txt, 10);
	TextOutA(window.context, 10, 100, "Balls", 5);
	TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

void ProcessInput()
{
	if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
	if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;

	if (!game.action && GetAsyncKeyState(VK_SPACE))
	{
		game.action = true;
	}
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
	HBITMAP hbm, hOldbm;
	HDC hMemDC;
	BITMAP bm;

	hMemDC = CreateCompatibleDC(hDC);
	hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);

	if (hOldbm)
	{
		GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm);

		if (alpha)
		{
			TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));
		}
		else
		{
			StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		}

		SelectObject(hMemDC, hOldbm);
	}

	DeleteDC(hMemDC);
}

void ShowRacketAndBall()
{
	ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);
	ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);
	ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);

	for (int y = 0; y < dimY; y++)
	{
		for (int x = 0; x < dimX; x++)
		{
			sprite& block = blocks[y][x];

			if (block.status)
			{
				ShowBitmap(window.context, block.x, block.y, block.width, block.height, block.hBitmap);
			}
		}
	}
}

void LimitRacket()
{
	racket.x = max(racket.x, racket.width / 2.);
	racket.x = min(racket.x, window.width - racket.width / 2.);
}

void CheckWalls()
{
	if (ball.x < ball.rad || ball.x > window.width - ball.rad)
	{
		ball.dx *= -1;
	}
}

void CheckRoof()
{
	if (ball.y < ball.rad)
	{
		ball.dy *= -1;
	}
}

bool tail = false;

void CheckFloor()
{
	if (ball.y > window.height - ball.rad - racket.height)
	{
		if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)
		{
			game.score++;
			ball.speed += 5. / game.score;
			ball.dy *= -1;

			if (racket.width > 10)
			{
				racket.width *= 0.975;
			}
		}
		else
		{
			tail = true;

			if (ball.y - ball.rad > window.height)
			{
				game.balls--;

				if (game.balls < 0) {

					MessageBoxA(window.hWnd, "game over", "", MB_OK);
					InitGame();
				}

				ball.dy = (rand() % 65 + 35) / 100.;
				ball.dx = -(1 - ball.dy);
				ball.x = racket.x;
				ball.y = racket.y - ball.rad;
				game.action = false;
				tail = false;
			}
		}
	}
}

bool collisionDetected = false; //для регистрации коллизии и дальнейшей ее обработки в методе ProcessBall()
float pathX = 0; //для калибровки положения шарика в методе ProcessBall()
float pathY = 0;
bool flipdx = false; //для выбора оси отражения в методе ProcessBall()
bool flipdy = false;

void CheckBlocks()
{
	bool isCollision = false;
	float fX = ball.x;
	float fY = ball.y;

	float deltaX = ball.dx * ball.speed;
	float deltaY = ball.dy * ball.speed;
	float L = sqrt(deltaX * deltaX + deltaY * deltaY);

	int points_count = 16;
	
	int lastBlockX = -1;
	int lastBlockY = -1;
		
	collisionDetected = false;
		

	for (float i = 0; i < L; i += 3) //L точек уже на луче
	{
		float alpha = PI / 2 + atan2(deltaY, deltaX); //угол первой точки
		float beta = PI / (points_count - 1); //угол смещения точек относительно центра шарика
		collisionDetected = false;

		for (int point = 0; point < points_count; point++) //points_count точек
		{

			if (collisionDetected) break;

			float angle = alpha - point * beta; //смещаем угол для новой точки


			float pointX = fX + ball.rad * cos(angle); //находим координаты новой точки
			float pointY = fY + ball.rad * sin(angle);

			float ddx = i / L * deltaX; //бежим последовательно по лучу
			float ddy = i / L * deltaY;

			SetPixel(window.context, pointX + ddx, pointY + ddy, RGB(255, 255, 127));

			for (int y = 0; y < dimY; y++) //строки блоков
			{
				if (collisionDetected) break;

				for (int x = 0; x < dimX; x++) //столбцы блоков
				{
					if (collisionDetected) break;


					if (blocks[y][x].status)
					{
						sprite& block = blocks[y][x]; //потенциальный для коллизии блок

						if ((pointX + ddx >= block.x) && (pointX + ddx <= block.x + block.width) && //x проверка коллизии
							(pointY + ddy >= block.y) && (pointY + ddy <= block.y + block.height) &&
							x != lastBlockX && y != lastBlockY) //y проверка коллизии
						{
							lastBlockX = x;
							lastBlockY = y;
							//block.status = false; //выключение блока

							collisionDetected = true; //сообщаем о коллизии

							float minX = min(block.x + block.width - (pointX + ddx), pointX + ddx - block.x);
							float minY = min(block.y + block.height - (pointY + ddy), pointY + ddy - block.y);

							if (minX < minY)
							{
								//pathY = deltaY;
								//pathX = 2 * ddx - deltaX;
								flipdx = true;

								if (deltaX < 0)
								{
									float d = pointX - (block.x + block.width);
									fX = pointX - d * 2;
								}
								else
								{
									float d = block.x - pointX;
									fX = pointX + d * 2;
								}

								deltaX *= -1;
							}
							else
							{
								//pathX = deltaX;
								//pathY = 2 * ddy - deltaY;
								flipdy = true;


								if (deltaY < 0)
								{
									float d = pointY - (block.y + block.height);
									fY = pointY - d * 2;
								}
								else
								{
									float d = block.y - pointY;
									fY = pointY + d * 2;
								}

								deltaY *= -1;
							}


							ddx = i / L * deltaX; //бежим последовательно по лучу
							ddy = i / L * deltaY;

						}
					}
				}
			}
		}
	}

	if (collisionDetected)
	{
	//	ball.x = fX;
	//	ball.y = fY;
	}
	else
	{
	//	ball.x += ball.dx * ball.speed;
	//	ball.y += ball.dy * ball.speed;
	}

	isCollision = false;
}

void ProcessRoom()
{
	CheckWalls();
	CheckRoof();
	CheckFloor();
	CheckBlocks(); //определяем, будет ли на следующем шаге столкновение с каким-либо блоком
}

void ProcessBall()
{
	if (game.action)
	{
		/*POINT mousePos;
		GetCursorPos(&mousePos);

		ball.x = mousePos.x;
		ball.y = mousePos.y;*/

		if (collisionDetected) //если будет коллизия, то выставляем правильные координаты
		{
			if (flipdx) ball.dx *= -1; //делаем отражение
			if (flipdy) ball.dy *= -1;

			ball.x += pathX; //корректируем позицию шарика
			ball.y += pathY;

			//сбрасываем вспомогательные переменные
			collisionDetected = false;
			flipdx = false;
			flipdy = false;

			return;
		} //иначе просто двигаем шарик как обычно
		ball.x += ball.dx * ball.speed;
		ball.y += ball.dy * ball.speed;
	}
	else
	{
		ball.x = racket.x;
	}
}

void InitWindow()
{
	SetProcessDPIAware();
	window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

	RECT r;
	GetClientRect(window.hWnd, &r);
	window.device_context = GetDC(window.hWnd);
	window.width = r.right - r.left;
	window.height = r.bottom - r.top;
	window.context = CreateCompatibleDC(window.device_context);
	SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));
	GetClientRect(window.hWnd, &r);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	InitWindow();
	InitGame();

	ShowCursor(NULL);

	while (!GetAsyncKeyState(VK_ESCAPE))
	{
		POINT mousePos;
		GetCursorPos(&mousePos);

		ball.x = mousePos.x;
		ball.y = mousePos.y;

		ShowRacketAndBall();
		ProcessRoom();
		BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);

		ShowScore();

		Sleep(17);

		ProcessInput();
		LimitRacket();
		
		//ProcessBall();
	}

	ReleaseDC(window.hWnd, window.context);
}
