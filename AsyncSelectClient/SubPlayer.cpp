#include "Player.h"
#include "SubPlayer.h"
#include <math.h>

extern bool g_isGameOver;

DWORD WINAPI SubPlayerThread(LPVOID arg)
{
	CPlayer *pSubPlayer = (CPlayer *)arg;
	CPlayer *pParentPlayer = pSubPlayer->GetParent();
	int pX, pY, sX, sY, offset;
	float angle = 0.0f;

	while (1)
	{
		pParentPlayer->GetPosition(pX, pY);
		pSubPlayer->GetPosition(sX, sY);

		offset = (int)(50.0f * sinf(angle));
		sY = pY + offset;
		pSubPlayer->SetPosition(sX, sY);

		angle += 1.0f;
		if (angle > 360.0f)
			angle = 0.0f;

		Sleep(200);
		if (g_isGameOver)
			break;
	}
	return 0;
}