// Fill out your copyright notice in the Description page of Project Settings.


#include "NetAvatar.h"
#include "GameFramework/CharacterMovementComponent.h"

ANetAvatar::ANetAvatar() :
    MovementScale(1.0f)
{
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    bIsSprinting = false;
}

void ANetAvatar::BeginPlay()
{
    Super::BeginPlay();

    Camera->bUsePawnControlRotation = false;
    SpringArm->bUsePawnControlRotation = true;
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ANetAvatar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("Turn", this, &ACharacter::AddControllerYawInput);
    PlayerInputComponent->BindAxis("Lookup", this, &ACharacter::AddControllerPitchInput);

    PlayerInputComponent->BindAxis("MoveForward", this, &ANetAvatar::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ANetAvatar::MoveRight);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ANetAvatar::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ANetAvatar::StopSprint);
}

void ANetAvatar::MoveForward(float Scale)
{
    FRotator Rotation = GetController()->GetControlRotation();
    FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
    FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    AddMovementInput(ForwardDirection, MovementScale*Scale);
}

void ANetAvatar::MoveRight(float Scale)
{
    FRotator Rotation = GetController()->GetControlRotation();
    FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
    FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    AddMovementInput(ForwardDirection, MovementScale*Scale);
}

void ANetAvatar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANetAvatar, bIsSprinting);
}
void ANetAvatar::StartSprint()
{
    ServerSetSprinting(true);
}

void ANetAvatar::StopSprint()
{
    ServerSetSprinting(false);
}

void ANetAvatar::ServerSetSprinting_Implementation(bool bNewSprinting)
{
    bIsSprinting = bNewSprinting;
    OnRep_IsSprinting();
}

void ANetAvatar::OnRep_IsSprinting()
{
    GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? 600.0f : 300.0f;
}

