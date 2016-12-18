#pragma once

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "myHeader.h"

#include <vector>
#include <list>

using namespace std;
using namespace Gdiplus;

typedef struct BULLET
{
	Rect rect;
	bool isBomb;

	BULLET()
	{
		rect.X = 0;
		rect.Y = 0;
		rect.Width = 5;
		rect.Height = 5;
		isBomb = false;
	}
}Bullet;

class CPlayer
{
protected:
	int      m_xPos;
	int      m_yPos;
	int      m_width;
	int      m_height;

	const int startHP = 20;
	int hp;

	Color  m_color;

	Rect   m_rect;
	SolidBrush  *m_brush;
	SolidBrush  *m_brushBullet;

	BOOL	m_isBullet;
	DWORD	m_tickBullet;
	BOOL     m_isBoom;
	DWORD  m_tickBoom;
	BOOL	m_isSuperGuard;
	DWORD	m_durationSuperGaurd;
	DWORD	m_tickSuperGuard;

	vector<Bullet *>  m_vecBullet;

	CPlayer		*m_pSubPlayer;
	CPlayer      *m_pParentPlayer;

public:
	unsigned int  m_Direction;

public:
	CPlayer(int x, int y, int width, int height, Color clr);
	virtual ~CPlayer();

	void Shoot();
	void AddBullet(int x, int y);
	void SetPosition(int xPos, int yPos);
	void Move(int xOffset, int yOffset);

	void Boom();
	void AddBoom(int x, int y);
	void SendSuperGuard();
	void BeginSuperGuard();
	void EndSuperGuard();

	BOOL Collision(CPlayer *other);
	BOOL Collision(Rect rect, CPlayer *other);

	void DrawBullet(Graphics *g, CPlayer *other);
	void Draw(Graphics *g, CPlayer *other, DWORD tick);
	void DrawUIHP(Graphics *g, Image* imgNum[]);

	void GameOver(bool isLeftWin);

	Rect GetRect() { return m_rect; }

	void GetPosition(int &x, int &y) { x = m_xPos; y = m_yPos; }


	void CreateSubPlayer();
	void SetParent(CPlayer *pPlayer) { m_pParentPlayer = pPlayer; }
	CPlayer *GetParent() { return m_pParentPlayer; }
	CPlayer *GetSubPlayer() { return m_pSubPlayer; }
};

#endif
