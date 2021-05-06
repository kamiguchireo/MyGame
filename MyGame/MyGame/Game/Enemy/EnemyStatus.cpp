#include "stdafx.h"
#include "EnemyStatus.h"
#include "Enemy.h"

void EnemyStatus::Damage(int i)
{
	m_hp -= i;
	if (m_hp <= 0)
	{
		Dead();
	}
}

void EnemyStatus::Dead()
{
	//死亡フラグを立てる
	m_en->Dead();
}