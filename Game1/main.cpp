/*
=================
main.cpp
Main entry point for the Card application
=================
*/

#include "GameConstants.h"
#include "GameResources.h"
#include "cD3DManager.h"
#include "cD3DXSpriteMgr.h"
#include "cD3DXTexture.h"
#include "cBalloon.h"
#include "cSprite.h"
#include "cExplosion.h"
#include "cXAudio.h"
#include "cD3DXFont.h"

using namespace std;
void SpawnEnemy(int currentEnemy, int ePos);
void SpawnNonMoveEnemy(int currentEnemy);

HINSTANCE hInst; // global handle to hold the application instance
HWND wndHandle; // global variable to hold the window handle
// Get a reference to the DirectX Manager
static cD3DManager* d3dMgr = cD3DManager::getInstance();
// Get a reference to the DirectX Sprite renderer Manager 
static cD3DXSpriteMgr* d3dxSRMgr = cD3DXSpriteMgr::getInstance();
D3DXVECTOR2 balloonTrans = D3DXVECTOR2(300,300);
vector<cBalloon*> aBalloon;
vector<cBalloon*>::iterator iter;
vector<cBalloon*>::iterator oldIter;
vector<cBalloon*>::iterator index;
vector<cBalloon*> aLazer;
vector<cBalloon*>::iterator lazeIter;
vector<cBalloon*>::iterator lazeIndex;
vector<cBalloon*>::iterator lazeErase;
RECT clientBounds;
TCHAR szTempOutput[30];
bool gHit = false;
int gBalloonsBurst = 0;
char gBalloonsBurstStr[50];
D3DXVECTOR3 expPos;
list<cExplosion*> gExplode;
cXAudio gExplodeSound;
cXAudio GameMusic;
cXAudio missleLaunch;
cD3DXTexture shipTex;
cD3DXTexture* balloonTextures[4];
char* balloonTxtres[] = {"Images\\Ship1.png","Images\\BalloonGreen.png","Images\\BalloonRed.png","Images\\explosion.png","Images\\Lazer.png",};
int spritePosX[] = {0,100,200,300,400,500,600,700,};
int spritePosY[] = { 0, 80, 10, 60, 80, 0, 60, 20, 50, 100, };
D3DXVECTOR3 shipPos;
D3DXVECTOR3 lazerPos;
D3DXVECTOR3 balloonPos;
D3DXVECTOR2 rocketTrans = D3DXVECTOR2(0, 0);
cBalloon player;
cBalloon lazer;
cD3DXTexture lazerTex;
D3DXVECTOR2 lazerTrans= D3DXVECTOR2(0,-200);
int lazerSpawn = 0;
bool lazerShoot = false;
int lazTime = 10;
int lazMax = 10;
float enemySpeed = 30.0f;
bool weapon;
int gameState = 1; // used in the game switch
D3DXVECTOR3 buttonPos;
cBalloon playButton;
float dt;
int currentEnemy = 0;
bool startUp = false;
bool menuStart = true;
int spawnTime = 0;
int spawnMax = 50;
int enemycounter = 0;
/*
==================================================================
* LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
* LPARAM lParam)
* The window procedure
==================================================================
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Check any available messages from the queue
	switch (message)
	{
		case WM_KEYDOWN:
			{
				if (wParam == VK_LEFT && shipPos.x >0 && gameState == 2)
				{
					player.setSpritePos(D3DXVECTOR3(shipPos.x -= 30.0f, clientBounds.bottom - 120.0f, 0.0f));
					return 0;
				}
				if (wParam == VK_RIGHT && shipPos.x <770 && gameState == 2)
				{
					player.setSpritePos(D3DXVECTOR3(shipPos.x += 30.0f, clientBounds.bottom - 120.0f, 0.0f));
					return 0;
				}
				if (wParam == VK_SPACE )
				{
					switch (gameState)
					{
					case 1:
						gameState = 4;
						break;
					case 2:
						if (lazTime == lazMax)	
						{
							lazerPos = D3DXVECTOR3(shipPos.x+25, shipPos.y, 0);  // starting position of lazer
							aLazer.push_back(new cBalloon());
							aLazer[lazerSpawn]->setSpritePos(lazerPos); // set sprite pos
							aLazer[lazerSpawn]->setTranslation(D3DXVECTOR2(0.0f, -200.0f)); // set translation direction and speed
							aLazer[lazerSpawn]->setTexture(new cD3DXTexture(d3dMgr->getTheD3DDevice(), "Images\\lazer.png")); // apply texture
							lazerSpawn++; // increment number of lazers
							lazTime--; // decrement lazer time
							missleLaunch.playSound(L"Sounds\\heavyLazer.wav", false); // play sound
						}
						break;
					case 3:
						break;
					case 4:
						break;
					default:
						break;
					}
					return 0;

					//return 0;
				}
				if (wParam == VK_RETURN)
				{
					switch (gameState)
					{
						case 1:
							gameState = 2;
							break;
						case 3:
							gBalloonsBurst = 0;
							gameState = 1;
							break;
						case 4:
							gameState = 2;
							break;
					default:
						break;
					}
					return 0;
				}
				if (wParam == VK_ESCAPE)
				{
					if (gameState == 3)
					{
						gameState = 1;
						return 0;
					}
					return 0;
				}
				return 0;
			}
		case WM_LBUTTONDOWN:
			{

				return 0;
			}
		case WM_CLOSE:
			{
			// Exit the Game
				PostQuitMessage(0);
				 return 0;
			}

		case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}
	}
	// Always return the message to the default window
	// procedure for further processing
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
==================================================================
* bool initWindow( HINSTANCE hInstance )
* initWindow registers the window class for the application, creates the window
==================================================================
*/
bool initWindow( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;
	// Fill in the WNDCLASSEX structure. This describes how the window
	// will look to the system
	wcex.cbSize = sizeof(WNDCLASSEX); // the size of the structure
	wcex.style = CS_HREDRAW | CS_VREDRAW; // the class style
	wcex.lpfnWndProc = (WNDPROC)WndProc; // the window procedure callback
	wcex.cbClsExtra = 0; // extra bytes to allocate for this class
	wcex.cbWndExtra = 0; // extra bytes to allocate for this instance
	wcex.hInstance = hInstance; // handle to the application instance
	wcex.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MyWindowIcon)); // icon to associate with the application
	wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_GUNSIGHT));// the default cursor
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); // the background color
	wcex.lpszMenuName = NULL; // the resource name for the menu
	wcex.lpszClassName = "Balloons"; // the class name being created
	wcex.hIconSm = LoadIcon(hInstance,"Balloon.ico"); // the handle to the small icon

	RegisterClassEx(&wcex);
	// Create the window
	wndHandle = CreateWindow("Balloons",			// the window class to use
							 "FTL: Super Mega Calum Brown Delux Edition",	// the title bar text
							WS_OVERLAPPEDWINDOW,	// the window style
							CW_USEDEFAULT, // the starting x coordinate
							CW_USEDEFAULT, // the starting y coordinate
							800, // the pixel width of the window
							600, // the pixel height of the window
							NULL, // the parent window; NULL for desktop
							NULL, // the menu for the application; NULL for none
							hInstance, // the handle to the application instance
							NULL); // no values passed to the window
	// Make sure that the window handle that is created is valid
	if (!wndHandle)
		return false;
	// Display the window on the screen
	ShowWindow(wndHandle, SW_SHOW);
	//ShowWindow(wndHandle, SW_MAXIMIZE);
	UpdateWindow(wndHandle);
	return true;
}

/*
==================================================================
// This is winmain, the main entry point for Windows applications
==================================================================
*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{

#pragma region GameInitStuff
	// Initialize the window
	if ( !initWindow( hInstance ) )
		return false;
	// called after creating the window
	if ( !d3dMgr->initD3DManager(wndHandle) )
		return false;
	if ( !d3dxSRMgr->initD3DXSpriteMgr(d3dMgr->getTheD3DDevice()))
		return false;

	// Grab the frequency of the high def timer
	__int64 freq = 0;				// measured in counts per second;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	float sPC = 1.0f / (float)freq;			// number of seconds per count

	__int64 currentTime = 0;				// current time measured in counts per second;
	__int64 previousTime = 0;				// previous time measured in counts per second;

	float numFrames   = 0.0f;				// Used to hold the number of frames
	float timeElapsed = 0.0f;				// cumulative elapsed time

	GetClientRect(wndHandle,&clientBounds);

	float fpsRate = 1.0f/30.0f;

	D3DXVECTOR3 aballoonPos;

	sprintf_s(gBalloonsBurstStr, 50, "SCORE : %d", gBalloonsBurst);

	//====================================================================
	// Load four textures for the balloons; yellow, green, red & explosion
	//====================================================================
	for (int txture = 0; txture < 4; txture++)
	{
		balloonTextures[txture] = new cD3DXTexture(d3dMgr->getTheD3DDevice(), balloonTxtres[txture]);
	}

	// Initial starting position for Rocket
	D3DXVECTOR3 balloonPos;

	/* initialize random seed: */
	srand ( (unsigned int)time(NULL) );
	/* generate random number of balloons */


	
	shipPos = D3DXVECTOR3(clientBounds.right/2-30.0f,clientBounds.bottom-120.0f,0); // starting positionof player;
	player.setTexture(new cD3DXTexture(d3dMgr->getTheD3DDevice(), "Images\\Ship.png"));
	player.setSpritePos(shipPos); 
	player.setTranslation(D3DXVECTOR2(0.0f,0.0f));

	LPDIRECT3DSURFACE9 aSurface;				// the Direct3D surface
	LPDIRECT3DSURFACE9 theBackbuffer = NULL;  // This will hold the back buffer
	
	MSG msg;
	ZeroMemory( &msg, sizeof( msg ) );

	// Create the background surface
	//aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\Nighttime.png");

	// load custom font
	cD3DXFont* balloonFont = new cD3DXFont(d3dMgr->getTheD3DDevice(),hInstance, "JustinFont12Bold");

	RECT textPos;
	SetRect(&textPos,0, 0, 600, 100);
	RECT menutextPos;
	SetRect(&menutextPos, clientBounds.right/2-50,clientBounds.bottom/2,clientBounds.right/2+300,clientBounds.bottom/2+100);
	//shipPos = D3DXVECTOR3(rocketTrans.x, clientBounds.bottom-120, 0);										//rocket stuff
	//player.setSpritePos(shipPos);

	QueryPerformanceCounter((LARGE_INTEGER*)&previousTime);
#pragma endregion GameInitStuff
	while( msg.message!=WM_QUIT )
	{
		// Check the message queue
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
		if (gameState == 1)
		{
			OutputDebugString("gameState == 1");
			if (menuStart)
			{
				GameMusic.playSound(L"Sounds\\MenuMusic.wav", true);
				menuStart = false;
				aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\FTLMENUBACKGROUND.png");
			}

			startUp = true;
			d3dMgr->beginRender();
			theBackbuffer = d3dMgr->getTheBackBuffer();
			d3dMgr->updateTheSurface(aSurface, theBackbuffer);
			d3dMgr->releaseTheBackbuffer(theBackbuffer);
			d3dxSRMgr->endDraw();
			d3dMgr->endRender();
		}
		
		else if(gameState == 2)
		{
#pragma region gameCode
				// Game code goes here
			OutputDebugString("gameState == 2");
			if (startUp)
			{
				aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\Nighttime.png"); // change the backrgound to the game background
				startUp = false; // set the startup bool to false so this only happens once
				enemySpeed = 30.0f; // sets the enemy speed to the default
				timeElapsed = 0.0f; // sets the time elapsed to 0
				currentEnemy = 0; // sets the currentEnemy to 0
				lazerSpawn = 0; // sets the current lazer to 0
				aBalloon.clear(); // clears the aBalloon vector
				aLazer.clear();// clears the aLazer vector to 0
				SpawnNonMoveEnemy(currentEnemy); // creates initial enemy
				gBalloonsBurst = 0; // sets the scire to 0
				sprintf_s(gBalloonsBurstStr, 50, "SCORE : %d", gBalloonsBurst); // displays the score
			}

			QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
			dt = (currentTime - previousTime)*sPC;
			// Accumulate how much time has passed.
			timeElapsed += dt;
			//lazer.setSpriteTransformMatrix(
			/*
			==============================================================
			| Update the postion of the balloons and check for collisions
			==============================================================
			*/
			if (timeElapsed > fpsRate)
			{
				if (lazTime < lazMax)lazTime--; // if lazTime is less than the max
				if (lazTime <= 0)lazTime = lazMax;
				spawnTime--;
				if (spawnTime <= 0)
				{
					spawnTime = spawnMax;
					SpawnEnemy(currentEnemy, rand()%8);
					currentEnemy++;
					enemySpeed += 1.0f;
					if (spawnMax >15)
					{
						spawnMax -= 1;
					}

				}
				for (iter = aBalloon.begin(); iter != aBalloon.end();)
					{
						OutputDebugString("Ship update");
						(*iter)->update(timeElapsed);			// update balloon
						aballoonPos = (*iter)->getSpritePos();  // get the position of the current balloon

						for (lazeIndex = aLazer.begin(); lazeIndex != aLazer.end();)
						{
							if ((*iter)->collidedWith((*iter)->getBoundingRect(), (*lazeIndex)->getBoundingRect()))	
							{
								gExplodeSound.playSound(L"Sounds\\explosion.wav", false);
								iter = aBalloon.erase(iter);
								lazeIndex = aLazer.erase(lazeIndex);
								currentEnemy--;
								lazerSpawn--;
								gBalloonsBurst+=10;
								sprintf_s(gBalloonsBurstStr, 50, "SCORE : %d", gBalloonsBurst);
							}
							else
							{
								++lazeIndex;
							}

						}
						if (aballoonPos.y > (clientBounds.bottom - 60))
						{
							gameState = 3;
						}
						++iter;
				}

				d3dMgr->beginRender();
				theBackbuffer = d3dMgr->getTheBackBuffer();
				d3dMgr->updateTheSurface(aSurface, theBackbuffer);
				d3dMgr->releaseTheBackbuffer(theBackbuffer);
				d3dxSRMgr->beginDraw();

				player.setTranslation(rocketTrans);
				d3dxSRMgr->setTheTransform(player.getSpriteTransformMatrix());
				d3dxSRMgr->drawSprite(player.getTexture(), NULL, NULL, NULL, 0XFFFFFFFF);																							// draw player &player.getSpritePos()
				d3dxSRMgr->setTheTransform(lazer.getSpriteTransformMatrix());

				d3dxSRMgr->drawSprite(lazer.getTexture(), NULL, NULL, NULL, 0XFFFFFFFF);																				// draw lazer &player.getSpritePos()
				lazer.setTranslation(lazerTrans);
				player.update(timeElapsed);
				lazer.update(timeElapsed);
				vector<cBalloon*>::iterator iterB = aBalloon.begin();
				for (iterB = aBalloon.begin(); iterB != aBalloon.end(); ++iterB)
				{
					d3dxSRMgr->setTheTransform((*iterB)->getSpriteTransformMatrix());
					d3dxSRMgr->drawSprite((*iterB)->getTexture(), NULL, NULL, NULL, 0xFFFFFFFF);

				}
				vector<cBalloon*>::iterator iterC = aLazer.begin();
				for (iterC = aLazer.begin(); iterC != aLazer.end(); ++iterC)
				{
					d3dxSRMgr->setTheTransform((*iterC)->getSpriteTransformMatrix());
					d3dxSRMgr->drawSprite((*iterC)->getTexture(), NULL, NULL, NULL, 0xFFFFFFFF);
					(*iterC)->update(timeElapsed);
				}
				list<cExplosion*>::iterator iter = gExplode.begin();
				while (iter != gExplode.end())
				{
					if ((*iter)->isActive() == false)
					{
						iter = gExplode.erase(iter);
					}
					else
					{
						(*iter)->update(timeElapsed);
						d3dxSRMgr->setTheTransform((*iter)->getSpriteTransformMatrix());
						d3dxSRMgr->drawSprite((*iter)->getTexture(), &((*iter)->getSourceRect()), NULL, NULL, 0xFFFFFFFF);
						++iter;
					}
				}
				d3dxSRMgr->endDraw();
				balloonFont->printText(gBalloonsBurstStr, textPos);
				d3dMgr->endRender();
				timeElapsed = 0.0f;
			}
			previousTime = currentTime;
#pragma endregion gameStuff
		}
		if (gameState == 3)
		{
			startUp = true;
			aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\MenuBack.png");
			sprintf_s(gBalloonsBurstStr, 50, "SCORE : %d", gBalloonsBurst);

			d3dMgr->beginRender();
			theBackbuffer = d3dMgr->getTheBackBuffer();
			d3dMgr->updateTheSurface(aSurface, theBackbuffer);
			d3dMgr->releaseTheBackbuffer(theBackbuffer);
			balloonFont->printText(gBalloonsBurstStr, menutextPos);
			d3dxSRMgr->endDraw();
			d3dMgr->endRender();

		}
		if (gameState == 4)
		{
			startUp = true;
			aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\FTLINSTRUCTIONS.png");
			d3dMgr->beginRender();
			theBackbuffer = d3dMgr->getTheBackBuffer();
			d3dMgr->updateTheSurface(aSurface, theBackbuffer);
			d3dMgr->releaseTheBackbuffer(theBackbuffer);
			d3dxSRMgr->endDraw();
			d3dMgr->endRender();

		}
	}
	}
	d3dxSRMgr->cleanUp();
	d3dMgr->clean();
	return (int) msg.wParam;

}

void SpawnNonMoveEnemy(int currentEnemy)
{
	aBalloon.push_back(new cBalloon());
	aBalloon[currentEnemy]->setSpritePos(D3DXVECTOR3 (10000000.0f,0.0f,0.0f)); // set sprite pos
	aBalloon[currentEnemy]->setTranslation(D3DXVECTOR2(0.0f, 0.0f)); // set translation direction and speed
	aBalloon[currentEnemy]->setTexture(balloonTextures[(rand() % 3)]); // set texture
}
void SpawnEnemy(int currentEnemy, int ePos)
{
	balloonPos = D3DXVECTOR3(spritePosX[ePos], clientBounds.top - spritePosY[ePos], 0.0f); // starting position of enemy
	aBalloon.push_back(new cBalloon());
	aBalloon[currentEnemy]->setSpritePos(balloonPos); // set sprite pos
	aBalloon[currentEnemy]->setTranslation(D3DXVECTOR2(0.0f, enemySpeed)); // set translation direction and speed
	aBalloon[currentEnemy]->setTexture(balloonTextures[(rand()%3)]); // set texture
}


#pragma region comment
/*
for(index = aBalloon.begin(); index != aBalloon.end(); ++index)
{
if ((*iter)->collidedWith((*iter)->getBoundingRect(),(*index)->getBoundingRect()))	// dont want this, this makes the enemies change direction if they collide
{
// if a collision occurs change the direction of each balloon that has collided
OutputDebugString("Collision!!");
(*iter)->setTranslation((*iter)->getTranslation()*(1));
(*index)->setTranslation((*index)->getTranslation()*(1));
}
}
*/
#pragma endregion comment


//	for (iter = aBalloon.begin(); iter != aBalloon.end();)
//	{
//	if (aballoonPos.y > (clientBounds.bottom - 60))
//	{
//		//gameState = 3;
//		EndGame();
//	}
//	else
//	{
//		++iter;
//	}
//	}

/*
POINT mouseXY;
mouseXY.x = LOWORD(lParam);
mouseXY.y = HIWORD(lParam);

expPos = D3DXVECTOR3((float)mouseXY.x,(float)mouseXY.y, 0.0f);

iter = aBalloon.begin();
while (iter != aBalloon.end() && !gHit)
{
if ( (*iter)->insideRect((*iter)->getBoundingRect(),mouseXY))
{
OutputDebugString("Hit!\n");
gHit = true;
expPos = (*iter)->getSpritePos();
gExplode.push_back(new cExplosion(expPos,balloonTextures[3]));
gExplodeSound.playSound(L"Sounds\\explosion.wav",false);
iter = aBalloon.erase(iter);
gBalloonsBurst++;
sprintf_s( gBalloonsBurstStr, 50, "Balloons Burst : %d", gBalloonsBurst);

}
else
{
++iter;
}
}
gHit = false;
*/