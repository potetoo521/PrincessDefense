// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCameraComponent.h"

// Pointsで与えられた座標群をNormal軸へ射影した結果のうち最小のものを取得します（ついでに大きさを考慮したオフセットを行います）。
float GetMinProjection(FVector Normal, const TArray<FVector>& Points, const TArray<float>& Sizes)
{
	float Retval = MAX_FLT;

	for (int i = 0; i < Points.Num(); ++i)
	{
		Retval = FMath::Min(Retval, FVector::DotProduct(Points[i], Normal) - Sizes[i]);
	}

	return Retval;
}

// ターゲットの位置・大きさおよびカメラの向きから最適なカメラの位置を決定
FVector CalcCameraLocationByTargetPoints(const TArray<FVector>& Points, const TArray<float>& Sizes, const FRotator CameraRotation, const float FovXDegrees, const float AspectRatio, const float FovLeftMargin, const float FovRightMargin, const float FovBottomMargin, const float FovTopMargin)
{
	const float HalfFov_X = FMath::DegreesToRadians(FovXDegrees * 0.5f);
	
	const float HalfTanX = FMath::Tan(HalfFov_X);
	const float HalfTanY = HalfTanX / AspectRatio;

	// 非対称なセーフフレームをサポート
	// セーフフレームの視野角を上下左右について求めます
	const float HalfFovLeft = FMath::Atan(HalfTanX * (1.0f - FovLeftMargin * 2.0f));
	const float HalfFovRight = FMath::Atan(HalfTanX * (1.0f - FovRightMargin * 2.0f));
	const float HalfFovBottom = FMath::Atan(HalfTanY * (1.0f - FovBottomMargin * 2.0f));
	const float HalfFovTop = FMath::Atan(HalfTanY * (1.0f - FovTopMargin * 2.0f));

	const float HalfFovLeftDeg = FMath::RadiansToDegrees(HalfFovLeft);
	const float HalfFovRightDeg = FMath::RadiansToDegrees(HalfFovRight);
	const float HalfFovBottomDeg = FMath::RadiansToDegrees(HalfFovBottom);
	const float HalfFovTopDeg = FMath::RadiansToDegrees(HalfFovTop);

	// 座標軸をカメラのそれと平行にすることで計算を簡略化できます。
	// 原点はそのまま、回転のみ合わせます
	// カメラの座標が原点ではないことに注意。
	TArray<FVector> PointsCS;
	PointsCS.SetNum(Points.Num());

	for (int i = 0; i < Points.Num(); ++i)
	{
		PointsCS[i] = CameraRotation.UnrotateVector(Points[i]);
	}

	// ここに計算結果を格納します
	FVector CameraLocationCS;


	// 左右の面による拘束
	float XbyLR;
	float Dydx;
	{
		// カメラと平行な座標系では、XY平面上で計算できます
		const FVector LeftBoundNormalCS = FVector::RightVector.RotateAngleAxis(-HalfFovLeftDeg, FVector::UpVector);
		const float LeftBoundPosCS = GetMinProjection(LeftBoundNormalCS, PointsCS, Sizes);

		const FVector RightBoundNormalCS = -FVector::RightVector.RotateAngleAxis(HalfFovRightDeg, FVector::UpVector);
		const float RightBoundPosCS = GetMinProjection(RightBoundNormalCS, PointsCS, Sizes);

		// 連立方程式を解きます。
		// Q は XY平面上に射影した左右の面の交点です
		// dot(Q, LeftBoundNormalCS) = LeftBoundPosCS
		// dot(Q, RightBoundNormalCS) = RightBoundPosCS
		// 
		// 整理して以下の式を得られます
		const float QX = (RightBoundNormalCS.Y * LeftBoundPosCS - LeftBoundNormalCS.Y * RightBoundPosCS) / (LeftBoundNormalCS.X * RightBoundNormalCS.Y - LeftBoundNormalCS.Y * RightBoundNormalCS.X);
		const float QY = (LeftBoundPosCS - QX * LeftBoundNormalCS.X) / LeftBoundNormalCS.Y;

		// 一時的に解を格納します。
		// Xは一時的な値で、上下の面の条件から得られる値との比較が必要です。
		// Yはこれで決まりです。
		// Zは上下面の条件を調べるまで分かりません。
		XbyLR = QX;
		CameraLocationCS.Y = QY;

		// Xに対するYの変化量（セーフフレーム非対称性への対処）を保存します
		Dydx = HalfTanX * (FovLeftMargin - FovRightMargin);
	}

	// 上下の面による拘束
	float XbyTB;
	float Dzdx;
	{
		// XZ平面上で計算できます
		const FVector BottomBoundNormalCS = FVector::UpVector.RotateAngleAxis(-HalfFovBottomDeg, -FVector::RightVector);
		const float BottomBoundPosCS = GetMinProjection(BottomBoundNormalCS, PointsCS, Sizes);

		const FVector TopBoundNormalCS = -FVector::UpVector.RotateAngleAxis(HalfFovTopDeg, -FVector::RightVector);
		const float TopBoundPosCS = GetMinProjection(TopBoundNormalCS, PointsCS, Sizes);

		// 左右面のときと同様です。
		const float QX = (TopBoundNormalCS.Z * BottomBoundPosCS - BottomBoundNormalCS.Z * TopBoundPosCS) / (BottomBoundNormalCS.X * TopBoundNormalCS.Z - BottomBoundNormalCS.Z * TopBoundNormalCS.X);
		const float QZ = (BottomBoundPosCS - QX * BottomBoundNormalCS.X) / BottomBoundNormalCS.Z;

		// 左右、上下の条件から得たXのうち小さい方（より後ろへ引いた方）が求める値です。
		XbyTB = QX;
		CameraLocationCS.Z = QZ;

		// Xに対するZの変化量（セーフフレーム非対称性への対処）を保存します
		Dzdx = HalfTanY * (FovBottomMargin - FovTopMargin);
	}

	CameraLocationCS.X = FMath::Min(XbyLR, XbyTB);
	CameraLocationCS.Y += (Dydx * (CameraLocationCS.X - XbyLR));
	CameraLocationCS.Z += (Dzdx * (CameraLocationCS.X - XbyTB));

	// World座標に復元して返します
	return CameraRotation.RotateVector(CameraLocationCS);
}

FVector GetLocation(FVector Location)
{

	return Location;
}