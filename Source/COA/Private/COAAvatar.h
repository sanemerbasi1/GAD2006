#pragma once

#include "CoreMinimal.h"
#include "COABaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "COAAvatar.generated.h"

//class UCameraComponent;
//class USpringArmComponent;

UCLASS()
class COA_API ACOAAvatar : public ACOABaseCharacter
{
    GENERATED_BODY()

public:
    ACOAAvatar();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float Stamina;

    UPROPERTY(EditAnywhere, Category = "Character")
	float MaxStamina;

    UPROPERTY(EditAnywhere, Category = "Character")
    float StaminaGainRate;

    UPROPERTY(EditAnywhere, Category = "Character")
    float StaminaDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    float RunSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	bool bStaminaDrained;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	bool bRunning;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    USpringArmComponent* SpringArm;

	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void RunPressed();
    void RunReleased();
    void UpdateMovementParams();

    public:
    virtual void Tick(float DeltaTime) override;

};