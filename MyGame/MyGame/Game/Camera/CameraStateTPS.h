#pragma once
#include "IGameCamera.h"
#include "CameraCollisionSolver.h"

class CameraStateTPS :public IGameCamera
{
public:
	CameraStateTPS();
	~CameraStateTPS();

	void Update(Vector3& pivotpos, float& rot);
	void SetCameraDist(float f)
	{
		CameraDist = f;
	}
	void SetAddPosY(float f)
	{
		AddPosY = f;
	}
private:
	float CameraDist = 200.0f;			//カメラからターゲットまでの距離
	float AddPosY = 0.0f;		//ポジションに加算する上方向の値
	const float MaxAddPos = 150.0f;		//ポジションに加算できる上方向の最大値
	const float MinAddPos = -50.0f;		//ポジションに加算できる上方向の最小値
	int DefaultPoint[2] = { 500,300 };
	float sensiX = 0.00025f;
	float sensiY = 0.1f;
	CameraCollisionSolver m_cameraCollisionSolver;
	float Col_Radius = 5.0f;		//スフィアコライダーの半径
};

