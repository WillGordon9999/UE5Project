//The purpose of this class is to provide a more reliable State Machine Solution to Blueprints
//As the component based solution is quite finnicky in terms of getting it to maintain references

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StateMachineSingleton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBPState);
DECLARE_DYNAMIC_DELEGATE(FBPHelper);

//Struct used for storing data about the previous state, when an interrupt state is triggered
//An example of this might be an AI moving towards a target location, a player throws a rock to draw the AI toward another goal,
//The AI returns to its previous task after finding nothing from investigating the rock.
USTRUCT(BlueprintType) struct FStateData
{
	GENERATED_BODY()

	FBPState EnterState;
	FBPState ExitState;
	FBPState UpdateState;
	UObject* Target;
};

//General struct containing information for the current object
USTRUCT(BlueprintType) struct FObjectState
{
	GENERATED_BODY()

	FBPState EnterState;
	FBPState ExitState;
	FBPState UpdateState;

	FBPState MainEnterState;
	FBPState MainExitState;
	FBPState MainUpdateState;
	bool returnToMainOnFail;

	UObject* target;

	TArray<FBPState> ActionList;
	int actionIndex;
	bool isListRunning;	
	bool returnToPreviousOnEnd;
		
	TArray<FStateData> stateData;
	
	FObjectState()
	{
		ActionList = TArray<FBPState>();
		stateData = TArray<FStateData>();
		actionIndex = 0;
		isListRunning = false;
		returnToPreviousOnEnd = false;
		returnToMainOnFail = false;
	}

};


UCLASS()
class NANITE_API AStateMachineSingleton : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AStateMachineSingleton();
	
	static TMap<UObject*, FObjectState> Objects;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
		
	UFUNCTION(BlueprintCallable, meta = (DisplayName="Change State By Names"))
	static void ChangeState(UObject* objectEntry, UObject* target, FName enterName, FName exitName, FName updateName);

	UFUNCTION(BlueprintCallable, meta = (DiplayName="Change State By Events"))
	static void ChangeStateByEvent(UObject* objectEntry, UObject* target, FBPHelper Enter, FBPHelper Exit, FBPHelper Update);

	UFUNCTION(BlueprintCallable, meta = (DiplayName = "Add Action To List By Names"))
	static void AddActionToList(UObject* objectEntry, UObject* target, FName functionName);

	UFUNCTION(BlueprintCallable, meta = (DiplayName = "Add Action To List By Event"))
	static void AddActionToListByEvent(UObject* objectEntry, UObject* target, FBPHelper function);

	UFUNCTION(BlueprintCallable)
	static void ChangeToNextActionInList(UObject* objectEntry);

	UFUNCTION(BlueprintCallable)
	static void StartActionList(UObject* objectEntry, bool returnToPreviousOnEnd, bool returnToMainOnFail);
	
	UFUNCTION(BlueprintCallable, meta = (DiplayName = "Set Main State By Names"))
	static void SetMainState(UObject* objectEntry, UObject* target, FName enterName, FName exitName, FName updateName);

	UFUNCTION(BlueprintCallable, meta = (DiplayName = "Set Main State By Events"))
	static void SetMainStateByEvent(UObject* objectEntry, UObject* target, FBPHelper enter, FBPHelper exit, FBPHelper update);

	UFUNCTION(BlueprintCallable)
	static void ChangeToMainState(UObject* objectEntry);

	UFUNCTION(BlueprintCallable, meta = (DiplayName = "Change To Interrupt By Names"))
	static void ChangeToInterrupt(UObject* objectEntry, UObject* target, FName enterName, FName exitName, FName updateName);

	UFUNCTION(BlueprintCallable, meta = (DiplayName = "Change To Interrupt By Events"))
	static void ChangeToInterruptByEvent(UObject* objectEntry, UObject* target, FBPHelper enter, FBPHelper exit, FBPHelper update);

	UFUNCTION(BlueprintCallable)
	static void ReturnToPreviousState(UObject* objectEntry, bool returnToMainOnFail);
};
