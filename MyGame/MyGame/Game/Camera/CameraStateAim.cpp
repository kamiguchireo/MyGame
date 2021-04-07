#include "stdafx.h"
#include "CameraStateAim.h"

CameraStateAim::CameraStateAim()
{

}

CameraStateAim::~CameraStateAim()
{

}

void CameraStateAim::Update(Vector3& pos, Vector3& target,float&rot)
{	
	//マウスカーソルの位置を取得
	POINT pt;
	GetCursorPos(&pt);
	int lstx = pt.x;
	int lsty = pt.y;

	//カメラの回転量をマウスカーソルの位置と初期値を比較して求める
	rot += (DefaultPoint[0] - lstx) * sensiX;
	//最低限の高さの確保
	target.y += DefaultCameraHeight;
	//最終的なカメラのポジション
	Vector3 m_pos = pos;
	//最終的なカメラのターゲットのポジション
	Vector3 targetPos = target;
	//カメラのポジションに足すY方向の値をマウスカーソルの位置と初期値を比較して求める
	*AddPosY -= (DefaultPoint[1] - lsty) * sensiY;

	*AddPosY = min(max(MinAddPos, *AddPosY), MaxAddPos);
	//カメラのポジションに足すベクトル
	Vector3 addPos = Vector3::Zero;
	//rotからカメラの位置を決める
	addPos.x -= sinf(rot) * CameraDist;
	addPos.y -= *AddPosY;
	addPos.z = cosf(rot) * CameraDist;
	targetPos += addPos;
	
	m_pos.y += DefaultCameraHeight;
	Vector3 AddPos = g_camera3D->GetForward();
	AddPos.y = 0.0f;
	AddPos.Normalize();
	m_pos += AddPos * 25.0f;

	//マウスカーソルの位置をセット
	SetCursorPos(500, 300);

	//カメラにセット
	g_camera3D->SetPosition(m_pos);
	g_camera3D->SetTarget(targetPos);
}