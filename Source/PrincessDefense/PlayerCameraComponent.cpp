// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCameraComponent.h"

// Points�ŗ^����ꂽ���W�Q��Normal���֎ˉe�������ʂ̂����ŏ��̂��̂��擾���܂��i���łɑ傫�����l�������I�t�Z�b�g���s���܂��j�B
float GetMinProjection(FVector Normal, const TArray<FVector>& Points, const TArray<float>& Sizes)
{
	float Retval = MAX_FLT;

	for (int i = 0; i < Points.Num(); ++i)
	{
		Retval = FMath::Min(Retval, FVector::DotProduct(Points[i], Normal) - Sizes[i]);
	}

	return Retval;
}

// �^�[�Q�b�g�̈ʒu�E�傫������уJ�����̌�������œK�ȃJ�����̈ʒu������
FVector CalcCameraLocationByTargetPoints(const TArray<FVector>& Points, const TArray<float>& Sizes, const FRotator CameraRotation, const float FovXDegrees, const float AspectRatio, const float FovLeftMargin, const float FovRightMargin, const float FovBottomMargin, const float FovTopMargin)
{
	const float HalfFov_X = FMath::DegreesToRadians(FovXDegrees * 0.5f);
	
	const float HalfTanX = FMath::Tan(HalfFov_X);
	const float HalfTanY = HalfTanX / AspectRatio;

	// ��Ώ̂ȃZ�[�t�t���[�����T�|�[�g
	// �Z�[�t�t���[���̎���p���㉺���E�ɂ��ċ��߂܂�
	const float HalfFovLeft = FMath::Atan(HalfTanX * (1.0f - FovLeftMargin * 2.0f));
	const float HalfFovRight = FMath::Atan(HalfTanX * (1.0f - FovRightMargin * 2.0f));
	const float HalfFovBottom = FMath::Atan(HalfTanY * (1.0f - FovBottomMargin * 2.0f));
	const float HalfFovTop = FMath::Atan(HalfTanY * (1.0f - FovTopMargin * 2.0f));

	const float HalfFovLeftDeg = FMath::RadiansToDegrees(HalfFovLeft);
	const float HalfFovRightDeg = FMath::RadiansToDegrees(HalfFovRight);
	const float HalfFovBottomDeg = FMath::RadiansToDegrees(HalfFovBottom);
	const float HalfFovTopDeg = FMath::RadiansToDegrees(HalfFovTop);

	// ���W�����J�����̂���ƕ��s�ɂ��邱�ƂŌv�Z���ȗ����ł��܂��B
	// ���_�͂��̂܂܁A��]�̂ݍ��킹�܂�
	// �J�����̍��W�����_�ł͂Ȃ����Ƃɒ��ӁB
	TArray<FVector> PointsCS;
	PointsCS.SetNum(Points.Num());

	for (int i = 0; i < Points.Num(); ++i)
	{
		PointsCS[i] = CameraRotation.UnrotateVector(Points[i]);
	}

	// �����Ɍv�Z���ʂ��i�[���܂�
	FVector CameraLocationCS;


	// ���E�̖ʂɂ��S��
	float XbyLR;
	float Dydx;
	{
		// �J�����ƕ��s�ȍ��W�n�ł́AXY���ʏ�Ōv�Z�ł��܂�
		const FVector LeftBoundNormalCS = FVector::RightVector.RotateAngleAxis(-HalfFovLeftDeg, FVector::UpVector);
		const float LeftBoundPosCS = GetMinProjection(LeftBoundNormalCS, PointsCS, Sizes);

		const FVector RightBoundNormalCS = -FVector::RightVector.RotateAngleAxis(HalfFovRightDeg, FVector::UpVector);
		const float RightBoundPosCS = GetMinProjection(RightBoundNormalCS, PointsCS, Sizes);

		// �A���������������܂��B
		// Q �� XY���ʏ�Ɏˉe�������E�̖ʂ̌�_�ł�
		// dot(Q, LeftBoundNormalCS) = LeftBoundPosCS
		// dot(Q, RightBoundNormalCS) = RightBoundPosCS
		// 
		// �������Ĉȉ��̎��𓾂��܂�
		const float QX = (RightBoundNormalCS.Y * LeftBoundPosCS - LeftBoundNormalCS.Y * RightBoundPosCS) / (LeftBoundNormalCS.X * RightBoundNormalCS.Y - LeftBoundNormalCS.Y * RightBoundNormalCS.X);
		const float QY = (LeftBoundPosCS - QX * LeftBoundNormalCS.X) / LeftBoundNormalCS.Y;

		// �ꎞ�I�ɉ����i�[���܂��B
		// X�͈ꎞ�I�Ȓl�ŁA�㉺�̖ʂ̏������瓾����l�Ƃ̔�r���K�v�ł��B
		// Y�͂���Ō��܂�ł��B
		// Z�͏㉺�ʂ̏����𒲂ׂ�܂ŕ�����܂���B
		XbyLR = QX;
		CameraLocationCS.Y = QY;

		// X�ɑ΂���Y�̕ω��ʁi�Z�[�t�t���[����Ώ̐��ւ̑Ώ��j��ۑ����܂�
		Dydx = HalfTanX * (FovLeftMargin - FovRightMargin);
	}

	// �㉺�̖ʂɂ��S��
	float XbyTB;
	float Dzdx;
	{
		// XZ���ʏ�Ōv�Z�ł��܂�
		const FVector BottomBoundNormalCS = FVector::UpVector.RotateAngleAxis(-HalfFovBottomDeg, -FVector::RightVector);
		const float BottomBoundPosCS = GetMinProjection(BottomBoundNormalCS, PointsCS, Sizes);

		const FVector TopBoundNormalCS = -FVector::UpVector.RotateAngleAxis(HalfFovTopDeg, -FVector::RightVector);
		const float TopBoundPosCS = GetMinProjection(TopBoundNormalCS, PointsCS, Sizes);

		// ���E�ʂ̂Ƃ��Ɠ��l�ł��B
		const float QX = (TopBoundNormalCS.Z * BottomBoundPosCS - BottomBoundNormalCS.Z * TopBoundPosCS) / (BottomBoundNormalCS.X * TopBoundNormalCS.Z - BottomBoundNormalCS.Z * TopBoundNormalCS.X);
		const float QZ = (BottomBoundPosCS - QX * BottomBoundNormalCS.X) / BottomBoundNormalCS.Z;

		// ���E�A�㉺�̏������瓾��X�̂������������i�����ֈ��������j�����߂�l�ł��B
		XbyTB = QX;
		CameraLocationCS.Z = QZ;

		// X�ɑ΂���Z�̕ω��ʁi�Z�[�t�t���[����Ώ̐��ւ̑Ώ��j��ۑ����܂�
		Dzdx = HalfTanY * (FovBottomMargin - FovTopMargin);
	}

	CameraLocationCS.X = FMath::Min(XbyLR, XbyTB);
	CameraLocationCS.Y += (Dydx * (CameraLocationCS.X - XbyLR));
	CameraLocationCS.Z += (Dzdx * (CameraLocationCS.X - XbyTB));

	// World���W�ɕ������ĕԂ��܂�
	return CameraRotation.RotateVector(CameraLocationCS);
}

FVector GetLocation(FVector Location)
{

	return Location;
}