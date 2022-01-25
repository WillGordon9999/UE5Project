


#include "StateMachine.h"


UStateMachine::UStateMachine()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	ActionList = TArray<FState>();
	DynamicActionList = TArray<FDynamicState>();
	isListRunning = false;
	isDynamicListRunning = false;
	actionIndex = 0;
	dynamicIndex = 0;
}


//Start
void UStateMachine::BeginPlay()
{
	Super::BeginPlay();			
}


//General Update
void UStateMachine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Run C++ State Sequence
	if (isListRunning)
	{
		if (actionIndex < ActionList.Num())
		{
			if (ActionList[actionIndex].IsBound())
				ActionList[actionIndex].Broadcast();
		}
	}

	//Run Blueprint-Friendly State Sequence
	else if (isDynamicListRunning)
	{
		if (dynamicIndex < DynamicActionList.Num())
		{
			if (DynamicActionList[dynamicIndex].IsBound())
				DynamicActionList[dynamicIndex].Broadcast();
		}
	}

	//Update Regular States Both C++ and Blueprint
	else
	{
		if (DynamicUpdateState.IsBound())
			DynamicUpdateState.Broadcast();

		if (UpdateState.IsBound())
			UpdateState.Broadcast();
	}	
}

//Start the list to be executed in a Behavior Tree-like Sequence
void UStateMachine::StartActionList()
{
	if (!isListRunning)
	{
		if (ActionList.Num() > 0)
		{
			isListRunning = true;
			actionIndex = 0;
		}
	}
}

//Changes to the next state in the Sequence, will return to normal state if there is no next state and will clear list
void UStateMachine::ChangeToNextActionInList()
{
	if (isListRunning)
	{
		actionIndex++;

		if (actionIndex >= ActionList.Num())
		{
			isListRunning = false;
			actionIndex = 0;
			ActionList.Empty();
			GEngine->AddOnScreenDebugMessage(0, 10.0f, FColor::Red, TEXT("Ending State List"));
		}
	}
}
