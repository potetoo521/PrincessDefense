// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "PlayerCameraComponent.generated.h"

/**
 * 
 */
UCLASS()
class PRINCESSDEFENSE_API UPlayerCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Cameraset")
		FVector GetLocation();

		float GetMinProjection(FVector Normal, const TArray<FVector>& Points, const TArray<float>& Sizes);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float Camera;
	
	

};
