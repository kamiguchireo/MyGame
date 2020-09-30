#pragma once
#include "SourceFile/graphic/Animation/AnimationClip.h"
#include "SourceFile/graphic/Animation/AnimationPlayController.h"

class Skeleton;

namespace Engine {
	class Animation
	{
	public:
		Animation()
		{
		}
		~Animation()
		{
		}

		//����������
		//skeleton		�X�P���g��
		//animClips		�A�j���[�V�����N���b�v
		void Init(Skeleton& skeleton, const std::vector<std::unique_ptr<AnimationClip>>& animClips);

		//�A�j���[�V�����̍Đ�
		//clipNo		�A�j���[�V�����N���b�v�̔ԍ�
		//interpolateTime		�⊮����
		void Play(int clipNo, float interpolateTime = 0.0f)
		{
			if (clipNo < m_animationClips.size())
			{
				PlayCommon(m_animationClips[clipNo], interpolateTime);
			}
		}

		//�A�j���[�V������i�߂�
		//�G���W�������Ŏg�p���܂�
		//deltaTime		�A�j���[�V������i�߂鎞��(�P�ʁF�b)
		void Progress(float deltaTime);

	private:
		////////////////////////////////////
		////////�A�j���[�V�����Đ��n////////
		////////////////////////////////////

		//�A�j���[�V�����̍Đ�
		void PlayCommon(AnimationClip* nextClip, float interpolateTime);


		//���[�J���|�[�Y�̍X�V
		//deltaTime		�A�j���[�V������i�߂鎞��(�P�ʁF�b)
		void UpdateLocalPose(float deltaTime);

		//�O���[�o���|�[�Y�̍X�V
		void UpdateGlobalPose();
	private:
		////////////////////////////////////////////////////
		////�A�j���[�V�����R���g���[���̃C���f�b�N�X�擾////
		////////////////////////////////////////////////////

		//�A�j���[�V�����R���g���[���̃����O�o�b�t�@��ł̃C���f�b�N�X���擾
		//startIndex		�J�n�C���f�b�N�X
		//localIndex		���[�J���C���f�b�N�X
		int GetAnimationControllerIndecx(int startIndex, int localIndex)const
		{
			return (startIndex + localIndex) % ANIMATION_PLAY_CONTROLLER_NUM;
		}
		//�ŏI�|�[�Y�ɂȂ�A�j���[�V�����̃����O�o�b�t�@��ł̃C���f�b�N�X���擾
		int GetLastAnimationControllerIndex()const
		{
			return GetAnimationControllerIndecx(m_startAnimationPlayController, m_numAnimationPlayController - 1);
		}
	private:
		static const int ANIMATION_PLAY_CONTROLLER_NUM = 32;	//!<�A�j���[�V�����R���g���[���̐��B
		Skeleton* m_skeleton = nullptr;		//�A�j���[�V������K�p����X�P���g��
		std::vector<AnimationClip*> m_animationClips;		//�A�j���[�V�����N���b�v�̔z��
		AnimationPlayController m_animationPlayController[ANIMATION_PLAY_CONTROLLER_NUM];		//�A�j���[�V�����v���C�R���g���[��
		int m_startAnimationPlayController = 0;		//�A�j���[�V�����R���g���[���̊J�n�C���f�b�N�X
		int m_numAnimationPlayController = 0;		//���ݎg�p���̃A�j���[�V�����Đ��R���g���[���̐�
		float m_interpolateTime = 0.0f;		//���݂̕⊮����
		float m_interpolateTimeEnd = 0.0f;		//�⊮�I������
		float m_deltaTimeOnUpdate = 0.0f;		//Update�֐������s�����Ƃ��̃f���^�^�C���B
		Vector3 m_footstepDeltaValue = g_vec3Zero;		//footstep�{�[���̈ړ���

	};
}