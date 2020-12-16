#include "stdafx.h"
#include "GameCamera.h"

GameCamera::GameCamera()
{
	//TPSカメラにする
	ChangeState(&TPScameraState);
}

GameCamera::~GameCamera()
{

}

bool GameCamera::Start()
{
	//g_camera3D->SetUp(m_up);
	return true;
}

void GameCamera::Update()
{
	currentState->Update(m_pos, m_target);
	g_camera2D->Update();
	g_camera3D->Update();

}