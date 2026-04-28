#include "COAAvatar.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ACOAAvatar::ACOAAvatar()

{
RunSpeed = 1000.0f;
Stamina = 100.0f;
MaxStamina = 100.0f;
StaminaDrainRate = 1.0f;
StaminaGainRate = 1.0f;
bStaminaDrained = false;
bRunning = false;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 300.0f;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    Camera ->bUsePawnControlRotation = false;
    SpringArm ->bUsePawnControlRotation = true;
    bUseControllerRotationYaw = false;
}

void ACOAAvatar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("LookUp", this, &ACharacter::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("Turn", this, &ACharacter::AddControllerYawInput);

    PlayerInputComponent->BindAxis("MoveForward", this, &ACOAAvatar::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACOAAvatar::MoveRight);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ACOAAvatar::RunPressed);
    PlayerInputComponent->BindAction("Run", IE_Released, this, &ACOAAvatar::RunReleased);
}

void ACOAAvatar::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ACOAAvatar::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ACOAAvatar::RunPressed()
{
    {
        bRunning = true;
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    }
}

void ACOAAvatar::RunReleased()
{
    {
    bRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
}

void ACOAAvatar::UpdateMovementParams()
{
    if (bRunning && !bStaminaDrained)
    {
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void ACOAAvatar::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking)
    {
        Stamina -= StaminaDrainRate * DeltaTime;
        if (Stamina <= 0.0f)
        {
            Stamina = 0.0f;
            bStaminaDrained = true;
            RunReleased();
            UpdateMovementParams();
        }
    }
    else
    {
        Stamina += StaminaGainRate * DeltaTime;
        if (Stamina >= MaxStamina)
        {
            Stamina = MaxStamina;
            bStaminaDrained = false;
            UpdateMovementParams();
        }
    }
}

