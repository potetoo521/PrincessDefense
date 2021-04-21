// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character_Master.h"
#include "Enemy_Character.generated.h"

UCLASS()
class PRINCESSDEFENSE_API AEnemy_Character : public ACharacter_Master
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	AEnemy_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
