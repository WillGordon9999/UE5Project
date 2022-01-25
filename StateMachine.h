
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StateMachine.generated.h"


/*
The goal of this class is to be a generic state machine component that any actor
could use and interact with by passing delegates/function pointers
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NANITE_API UStateMachine : public UActorComponent
{
	GENERATED_BODY()

public:		
	UStateMachine();

	//Declare Unreal Delegate Types needed	
	DECLARE_MULTICAST_DELEGATE(FState);


protected:	
	virtual void BeginPlay() override;

	//Standard Multi-Cast Delegates
	FState EnterState;
	FState ExitState;
	FState UpdateState;	

	//Action List of Delegates, similar to sequences in Behavior Trees 
	TArray<FState> ActionList;	
	int actionIndex;
	bool isListRunning;
		
public:	
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//Change state by passing delegates from object type T to be called on the target
	//Planning extra overloads to allow functions from multiple objects
	template <typename T>
	void ChangeState(T* target, void(T::* enter)(), void(T::* exit)(), void(T::* update)())
	{
		if (ExitState.IsBound())
			ExitState.Broadcast();

		EnterState.Clear();
		ExitState.Clear();
		UpdateState.Clear();

		EnterState.AddUObject(target, enter);
		ExitState.AddUObject(target, exit);
		UpdateState.AddUObject(target, update);

		if (EnterState.IsBound())
			EnterState.Broadcast();
	}

	//Add a function to list to be executed in a Sequence
	template <typename T>
	void AddActionToList(T* target, void(T::* function)())
	{
		FState actionHelper;
		actionHelper.AddUObject(target, function);
		ActionList.Add(actionHelper);
	}

	//Start the list to be executed in a Sequence
	void StartActionList();
	
	//Changes to the next action in the Sequence, will return to normal state if there is no next action and will clear the list
	void ChangeToNextActionInList();		
};
