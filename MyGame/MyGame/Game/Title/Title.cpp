#include "stdafx.h"
#include "Title.h"
#include "SourceFile/level/Level.h"
#include "Game/Stage/Tree/Tree1.h"
#include "Game/Stage/Tree/Tree2.h"

Title::~Title()
{

}

void Title::OnDestroy()
{
	//���x�����폜
	if (m_level != nullptr)
	{
		delete m_level;
		m_level = nullptr;
	}
	//�X�L�����f�����폜
	if (m_skinModel != nullptr)
	{
		DeleteGO(m_skinModel);
		m_skinModel = nullptr;
	}
	//������폜
	if (m_weapon != nullptr)
	{
		DeleteGO(m_weapon);
		m_weapon = nullptr;
	}
	//�؂��폜
	if (m_Tree1 != nullptr)
	{
		DeleteGO(m_Tree1);
		m_Tree1 = nullptr;
	}
	if (m_Tree2 != nullptr)
	{
		DeleteGO(m_Tree2);
		m_Tree2 = nullptr;
	}
	//�T�E���h
	if (m_FireSound != nullptr)
	{
		DeleteGO(m_FireSound);
		m_FireSound = nullptr;
	}
	//�X�v���C�g
	if (m_sprite != nullptr)
	{
		DeleteGO(m_sprite);
		m_sprite = nullptr;
	}
}

bool Title::Start()
{
	if (m_level == nullptr)
	{
		m_level = new Level();
	}

	m_level->Init("Assets/Level/Title.tkl", [&](const LevelObjectData& objData) {
		if (wcscmp(objData.name, L"Soldier_bs01") == 0)
		{
			m_pos = objData.position;
			m_rot = objData.rotation;
			m_skinModel = NewGO<prefab::ModelRender>(0);
			//Tkm�t�@�C�����Z�b�g
			m_skinModel->SetTkmFilePath("Assets/modelData/soldier_bs01.tkm");
			//���_�V�F�[�_�[�̃G���g���[�|�C���g
			m_skinModel->SetVSEntryPoint("VSMainSkin");
			//�X�P���g�����Z�b�g
			m_skinModel->SetSkeleton(m_skeleton);
			//�X�P���g���ƃA�j���[�V�����̏�����
			m_skeleton.Init("Assets/modelData/soldier_bs01.tks");
			//�X�P���g���̃A�b�v�f�[�g
			m_skeleton.Update(Matrix::Identity);
			//�����NewGO
			m_weapon = NewGO<Weapon>(1);
			m_weapon->Init(&m_skeleton, false);
			//�ʒu���Z�b�g
			m_skinModel->SetPosition(m_pos);
			//��]���Z�b�g
			m_skinModel->SetRotation(m_rot);
			//�A�j���[�V�������Z�b�g
			m_animClip[enTitleCharacterAnimation_Rifle_Idle].Load("Assets/animData/Rifle_Idle.tka");
			m_animClip[enTitleCharacterAnimation_Rifle_Idle].SetLoopFlag(false);
			m_animClip[enTitleCharacterAnimation_Rifle_Down_To_Aim].Load("Assets/animData/Rifle_Down_To_Aim.tka");
			m_animClip[enTitleCharacterAnimation_Rifle_Down_To_Aim].SetLoopFlag(false);

			//�A�j���[�V������������
			m_animation.Init(m_skeleton, m_animClip, enTitleCharacterAnimation_Num);
			m_animation.Play(enTitleCharacterAnimation_Rifle_Idle);

			return true;
		}
		else if (wcscmp(objData.name, L"SM_Tree_Tropic_02_novines") == 0)
		{
			//��
			if (m_Tree1 == nullptr)
			{
				m_Tree1 = NewGO<Tree1>(0);
				m_Tree1->InitInstance(objData.position, objData.rotation, objData.scale);
			}
			else
			{
				m_Tree1->AddInstance(objData.position, objData.rotation, objData.scale);
			}
			return true;
		}
		else if (wcscmp(objData.name, L"SM_Tree_Tropic_02") == 0)
		{
			//��
			if (m_Tree2 == nullptr)
			{
				m_Tree2 = NewGO<Tree2>(0);
				m_Tree2->InitInstance(objData.position, objData.rotation, objData.scale);
			}
			else
			{
				m_Tree2->AddInstance(objData.position, objData.rotation, objData.scale);
			}
			return true;
		}
		return false;
	});


	m_FireSound = NewGO<SoundSource>(0);
	m_FireSound->Init(L"Assets/sound/Anti_materiel_rifle.wav");

	m_sprite = NewGO<prefab::SpriteRender>(0);
	m_sprite->Init("Assets/image/BulletTitle.dds",250, 250);
	m_sprite->SetPosition(Vector3::Zero);
	m_sprite->SetAlpha(0.0f);

	m_CameraPos = { 200.0f,150.0f,200.0f };
	m_CameraTarget = { 0.0f,150.0f,0.0f };
	return true;
}

void Title::CameraMove()
{
	//�⊮��̃J�����̐ݒ�
	Vector3 NextCameraPos = { 100.0f,15.0f,350 };
	Vector3 NextCameraTarget = { 0.0f,50.0f,0.0f };
	Vector3 NextCameraUp = { -0.5f,0.0f,-0.5f };

	//�⊮��
	static float interpolate = 0.0f;
	interpolate += g_gameTime.GetFrameDeltaTime() * 0.2f;
	//�⊮�����悷��
	float f = pow(interpolate, 2.0);
	
	//�ő�l��1�ɂ���
	f = min(1.0f, f);

	m_CameraPos.Lerp(f, m_CameraPos, NextCameraPos);
	m_CameraTarget.Lerp(f, m_CameraTarget, NextCameraTarget);
	m_CameraUp.Lerp(f, m_CameraUp, NextCameraUp);
	g_camera3D->SetUp(m_CameraUp);
}

void Title::Update()
{
	auto deltatime = g_gameTime.GetFrameDeltaTime();
	if (GetAsyncKeyState(VK_LBUTTON))
	{
		m_animation.Play(enTitleCharacterAnimation_Rifle_Down_To_Aim, 0.3f);
		IsEnterGame = true;
	}
	if (IsEnterGame)
	{
		if (m_animation.IsPlaying() == false)
		{
			if (IsPlayFireSound == false)
			{
				m_FireSound->Play(false, false);
				m_sprite->SetAlpha(1.0f);
				IsPlayFireSound = true;
			}
			else
			{
				CameraMove();
			}
		}
	}

	m_animation.Update(deltatime);

	g_camera3D->SetPosition(m_CameraPos);
	g_camera3D->SetTarget(m_CameraTarget);
	m_skinModel->SetPosition(m_pos);
	m_skinModel->SetRotation(m_rot);
}