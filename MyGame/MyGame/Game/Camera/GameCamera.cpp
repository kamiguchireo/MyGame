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
	switch (m_StateNum)
	{
	case CameraState::TPS:
		//TPSカメラにする
		ChangeState(&TPScameraState);
		break;
	case CameraState::AIM:
		//AIMカメラにする
		ChangeState(&AIMcameraState);
		break;
	default:
		break;
	}
	currentState->SetAddPosY(&AddPosY);
	currentState->Update(m_pivotPos,rot);
	g_camera2D->Update();
	g_camera3D->Update();

}