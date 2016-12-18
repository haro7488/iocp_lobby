#include "Player.h"

#include "SubPlayer.h"

extern SOCKET g_sock;
extern bool g_isGameOver;
extern bool g_isLeftWin;

#define BULLET_DELAY 150
#define BOOM_DELAY	3000
#define SUPER_GUARD_DURATION	2000
#define SUPER_GUARD_DELAY	5000

CPlayer::CPlayer(int x, int y, int width, int height, Color clr) : m_xPos(x), m_yPos(y), m_width(width), m_height(height)
{
	m_rect.X = x;
	m_rect.Y = y;
	m_rect.Width = width;
	m_rect.Height = height;

	m_color = clr;

	m_brush = new SolidBrush(clr);
	m_brushBullet = new SolidBrush(clr);

	m_Direction = DIRECTION_LEFT;

	m_isBullet = TRUE;
	m_tickBullet = BULLET_DELAY;

	m_isBoom = TRUE;
	m_tickBoom = BOOM_DELAY;

	m_isSuperGuard = TRUE;
	m_durationSuperGaurd = SUPER_GUARD_DURATION;
	m_tickSuperGuard = SUPER_GUARD_DELAY;

	m_pSubPlayer = NULL;
	m_pParentPlayer = NULL;

	hp = startHP;
}


CPlayer::~CPlayer()
{
	delete m_brush;
	delete m_brushBullet;

	vector<Bullet *>::iterator it = m_vecBullet.begin();
	for (; it < m_vecBullet.end(); it++)
	{
		delete (*it);
	}

	m_vecBullet.clear();

	delete m_pSubPlayer; m_pSubPlayer = NULL;
}


void CPlayer::AddBullet(int x, int y)
{
	Bullet *bullet = new Bullet();

	bullet->rect.X = x;
	bullet->rect.Y = y;

	m_vecBullet.push_back(bullet);
}

void CPlayer::Shoot()
{
	if (m_tickBullet < BULLET_DELAY)
		return;

	Bullet *bullet = new Bullet();

	bullet->rect.X = m_xPos;
	bullet->rect.Y = m_yPos + 25;

	m_vecBullet.push_back(bullet);

	POSITIONINFO posInfo;
	posInfo.x = m_xPos;
	posInfo.y = m_yPos + 25;
	posInfo.PktID = PKT_SHOOT;
	posInfo.PktSize = sizeof(posInfo);

	int ret = send(g_sock, (char*)&posInfo, posInfo.PktSize, 0);
	if (ret == SOCKET_ERROR)
	{

	}

	m_isBullet = FALSE;
	m_tickBullet = 0;
}

void CPlayer::Boom()
{
	if (m_tickBoom < BOOM_DELAY)
		return;

	Bullet *bullet = new Bullet();

	bullet->isBomb = true;

	bullet->rect.X = m_xPos;
	bullet->rect.Y = m_yPos + 25;
	bullet->rect.Width = bullet->rect.Height = 15;

	m_vecBullet.push_back(bullet);

	POSITIONINFO posInfo;
	posInfo.x = m_xPos;
	posInfo.y = m_yPos + 25;
	posInfo.PktID = PKT_BOOM;
	posInfo.PktSize = sizeof(posInfo);

	int ret = send(g_sock, (char*)&posInfo, posInfo.PktSize, 0);
	if (ret == SOCKET_ERROR)
	{

	}

	m_isBoom = FALSE;
	m_tickBoom = 0;
}


void CPlayer::AddBoom(int x, int y)
{
	Bullet *bullet = new Bullet();

	bullet->isBomb = true;

	bullet->rect.X = x;
	bullet->rect.Y = y;
	bullet->rect.Width = bullet->rect.Height = 15;

	m_vecBullet.push_back(bullet);
}

void CPlayer::SendSuperGuard()
{
	PACKETHEADER superGuard;
	superGuard.PktID = PKT_ENEMYSUPERGUARD;
	superGuard.PktSize = sizeof(superGuard);
	int ret = send(g_sock, (char*)&superGuard, superGuard.PktSize, 0);
	if (ret == SOCKET_ERROR)
	{

	}
}

void CPlayer::BeginSuperGuard()
{
	if (m_tickSuperGuard < SUPER_GUARD_DELAY)
		return;

	GetSubPlayer()->m_rect.Height = 100;
	m_isSuperGuard = FALSE;
	m_tickSuperGuard = 0;
	m_durationSuperGaurd = 0;
}

void CPlayer::EndSuperGuard()
{
	GetSubPlayer()->m_rect.Height = 25;
}


void CPlayer::SetPosition(int xPos, int yPos)
{
	m_xPos = xPos;
	m_yPos = yPos;

	m_rect.X = m_xPos;
	m_rect.Y = m_yPos;
}

void CPlayer::Move(int xOffset, int yOffset)
{
	if (m_yPos + yOffset > 600 - m_rect.Height || m_yPos + yOffset < 60)
		return;

	m_xPos += xOffset;
	m_yPos += yOffset;

	m_rect.X = m_xPos;
	m_rect.Y = m_yPos;

	POSITIONINFO posInfo;
	posInfo.x = m_xPos;
	posInfo.y = m_yPos;
	posInfo.PktID = PKT_MOVE;
	posInfo.PktSize = sizeof(posInfo);

	int ret = send(g_sock, (char*)&posInfo, posInfo.PktSize, 0);
	if (ret == SOCKET_ERROR)
	{

	}
}


void CPlayer::DrawBullet(Graphics *g, CPlayer *other)
{
	Bullet *bullet;

	list<vector<Bullet *>::iterator> lstDelete;

	vector<Bullet *>::iterator it = m_vecBullet.begin();
	for (int i = 0; it < m_vecBullet.end(); it++, i++)
	{
		bullet = (*it);


		g->FillRectangle(m_brushBullet, bullet->rect);


		if (Collision(bullet->rect, other))
		{
			lstDelete.push_front(it);
			if (bullet->isBomb)
				other->hp -= 3;
			else
				other->hp--;

			if (other->hp < 0 && !g_isGameOver)
			{
				if (m_Direction == DIRECTION_LEFT)
					GameOver(true);
				else
					GameOver(false);
			}
			continue;
		}

		if (Collision(bullet->rect, other->GetSubPlayer()))
		{
			if (bullet->isBomb)
				continue;

			lstDelete.push_back(it);
			continue;
		}

		if (m_Direction == DIRECTION_LEFT)
		{
			bullet->rect.X += 5;
			if (bullet->rect.X > 1000)
				lstDelete.push_front(it);
		}
		else
		{
			bullet->rect.X -= 5;
			if (bullet->rect.X < 0)
				lstDelete.push_front(it);
		}
	}



	for (unsigned int deleteIndex = 0; deleteIndex < lstDelete.size(); deleteIndex++)
	{
		it = lstDelete.front();
		lstDelete.pop_front();

		if (*it != nullptr)
			delete (*it);
		m_vecBullet.erase(it);
	}

	lstDelete.clear();
}




void CPlayer::Draw(Graphics *g, CPlayer *other, DWORD tick)
{
	DrawBullet(g, other);

	if (!m_isBullet)
	{
		m_tickBullet += tick;

		if (m_tickBullet > BULLET_DELAY)
		{
			m_isBullet = TRUE;
		}
	}

	if (!m_isBoom)
	{
		m_tickBoom += tick;

		float value = (float)m_tickBoom / BOOM_DELAY;
		if (value > 1.0f)
		{
			m_isBoom = TRUE;
			value = 1.0f;
		}

		m_brush->SetColor(Gdiplus::Color(m_color.GetR() * value, m_color.GetG() * value, m_color.GetB() * value));
	}

	if (!m_isSuperGuard)
	{
		m_tickSuperGuard += tick;
		m_durationSuperGaurd += tick;

		float value = (float)m_durationSuperGaurd / SUPER_GUARD_DURATION;
		if (value > 1.0f)
		{
			value = 1.0f;
			EndSuperGuard();
		}

		if (m_tickSuperGuard > SUPER_GUARD_DELAY)
		{
			m_isSuperGuard = TRUE;
		}

		CPlayer* subPlayer = GetSubPlayer();
		subPlayer->m_brush->SetColor(Gdiplus::Color(subPlayer->m_color.GetR() * value, subPlayer->m_color.GetG() * value, subPlayer->m_color.GetB() * value));
	}

	g->FillRectangle(m_brush, m_rect);

	if (m_pSubPlayer != NULL)
		m_pSubPlayer->Draw(g, other, tick);
}

void CPlayer::DrawUIHP(Graphics * g, Image * imgNum[])
{
	int xPos = 0;
	if (m_Direction == DIRECTION_LEFT)
	{
		xPos = 20;
	}
	else
	{
		xPos = 1000 - 20 - 40 - 35;
	}

	int ten = 0;
	int one = 0;
	if (hp != 0)
	{
		ten = hp / 10;
		one = hp % 10;
	}
	if (one < 0 || ten < 0)
	{
		ten = 0;
		one = 0;
	}

	g->DrawImage(imgNum[ten], xPos, 10, 40, 40);
	g->DrawImage(imgNum[one], xPos + 35, 10, 40, 40);
}

void CPlayer::GameOver(bool isLeftWin)
{
	g_isGameOver = true;
	g_isLeftWin = isLeftWin;
}

BOOL CPlayer::Collision(Rect rect, CPlayer *other)
{
	Rect rc, rc1;

	rc1 = other->GetRect();

	if (Rect::Intersect(rc, rect, rc1))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CPlayer::Collision(CPlayer *other)
{
	Rect rc, rc1;
	rc1 = other->GetRect();

	if (Rect::Intersect(rc, m_rect, rc1))
	{
		return TRUE;
	}

	return FALSE;
}

void CPlayer::CreateSubPlayer()
{
	if (m_pSubPlayer != NULL)
		return;

	int x = m_xPos;
	int y = m_yPos;
	if (m_Direction == DIRECTION_LEFT)
	{
		x += 100 + 25;
	}
	else
	{
		x -= 100;
	}


	m_pSubPlayer = new CPlayer(x, y, 25, 25, m_color);
	m_pSubPlayer->SetParent(this);

	DWORD idThread;
	HANDLE hThread = CreateThread(NULL, 0, SubPlayerThread, (LPVOID)m_pSubPlayer, 0, &idThread);
	if (hThread != NULL)
		CloseHandle(hThread);


}