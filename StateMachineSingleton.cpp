
#include "StateMachineSingleton.h"

TMap<UObject*, FObjectState> AStateMachineSingleton::Objects = TMap<UObject*, FObjectState>();

// Sets default values
AStateMachineSingleton::AStateMachineSingleton()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AStateMachineSingleton::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStateMachineSingleton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Objects.Num() > 0)
	{
		for (auto& Elem : Objects)
		{
			if (Elem.Value.isListRunning)
			{
				if (Elem.Value.actionIndex < Elem.Value.ActionList.Num())
				{
					if (Elem.Value.ActionList[Elem.Value.actionIndex].IsBound())
						Elem.Value.ActionList[Elem.Value.actionIndex].Broadcast();
				}
			}

			else
			{
				if (Elem.Value.UpdateState.IsBound())
					Elem.Value.UpdateState.Broadcast();
			}
		}
	}
}

//Change state for the given objectEntry, accessing functions by names from target
void AStateMachineSingleton::ChangeState(UObject* objectEntry, UObject* target, FName enterName, FName exitName, FName updateName)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{
		if (state->ExitState.IsBound())
			state->ExitState.Broadcast();

		FBPHelper enter = FBPHelper();
		FBPHelper exit = FBPHelper();
		FBPHelper update = FBPHelper();

		if (state->stateData.Num() > 0)
			state->stateData.Empty();

		state->EnterState.Clear();
		state->ExitState.Clear();
		state->UpdateState.Clear();

		enter.BindUFunction(target, enterName);
		exit.BindUFunction(target, exitName);
		update.BindUFunction(target, updateName);

		state->EnterState.Add(enter);
		state->ExitState.Add(exit);
		state->UpdateState.Add(update);

		if (state->EnterState.IsBound())
			state->EnterState.Broadcast();
	}

	else
	{
		FObjectState newState = FObjectState();

		FBPHelper enter = FBPHelper();
		FBPHelper exit = FBPHelper();
		FBPHelper update = FBPHelper();

		enter.BindUFunction(target, enterName);
		exit.BindUFunction(target, exitName);
		update.BindUFunction(target, updateName);

		newState.EnterState.Add(enter);
		newState.ExitState.Add(exit);
		newState.UpdateState.Add(update);

		Objects.Add(objectEntry, newState);

		if (newState.EnterState.IsBound())
			newState.EnterState.Broadcast();
	}
}

//Change state for the given objectEntry, accessing functions by function delegates from target (Using the Create Event Node is Recommended for this)
void AStateMachineSingleton::ChangeStateByEvent(UObject* objectEntry, UObject* target, FBPHelper Enter, FBPHelper Exit, FBPHelper Update)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{
		if (state->ExitState.IsBound())
			state->ExitState.Broadcast();
		
		if (state->stateData.Num() > 0)
			state->stateData.Empty();

		state->EnterState.Clear();
		state->ExitState.Clear();
		state->UpdateState.Clear();
		
		state->EnterState.Add(Enter);
		state->ExitState.Add(Exit);
		state->UpdateState.Add(Update);

		if (state->EnterState.IsBound())
			state->EnterState.Broadcast();
	}

	else
	{
		FObjectState newState = FObjectState();
	
		newState.EnterState.Add(Enter);
		newState.ExitState.Add(Exit);
		newState.UpdateState.Add(Update);

		Objects.Add(objectEntry, newState);

		if (newState.EnterState.IsBound())
			newState.EnterState.Broadcast();
	}
}

//Add an action to be executed in sequence one at a time, function is to be added using name from object target
void AStateMachineSingleton::AddActionToList(UObject* objectEntry, UObject* target, FName functionName)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{	
		FBPHelper helper = FBPHelper();
		helper.BindUFunction(target, functionName);

		FBPState newState = FBPState();
		newState.Add(helper);

		state->ActionList.Add(newState);		
	}	
}

//Add an action to be executed in sequence one at a time, function is to be added using delegate from object target
void AStateMachineSingleton::AddActionToListByEvent(UObject* objectEntry, UObject* target, FBPHelper function)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{		
		FBPState newState = FBPState();
		newState.Add(function);

		state->ActionList.Add(newState);
	}
}

//Function to call in specific functions designed to be put into the Action List to tell the state machine to advance
void AStateMachineSingleton::ChangeToNextActionInList(UObject* objectEntry)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{
		if (state->isListRunning)
		{
			state->actionIndex++;

			if (state->actionIndex >= state->ActionList.Num())
			{
				state->isListRunning = false;
				state->actionIndex = 0;
				state->ActionList.Empty();

				if (state->returnToPreviousOnEnd)
				{
					state->returnToPreviousOnEnd = false;
					ReturnToPreviousState(objectEntry, state->returnToMainOnFail);
				}
			}
		}
	}
}

//Start the list of Actions to execute in sequence, can choose how to return after sequence is complete
void AStateMachineSingleton::StartActionList(UObject* objectEntry, bool returnToPreviousOnEnd, bool returnToMainOnFail)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{
		if (!state->isListRunning)
		{
			if (state->ActionList.Num() > 0)
			{
				state->isListRunning = true;
				state->actionIndex = 0;
				state->returnToPreviousOnEnd = returnToPreviousOnEnd;
				state->returnToMainOnFail = returnToMainOnFail;
			}
		}
	}
}

//The specific version of Change State to call when you want the objectEntry to return to its previous state after this state is complete 
//Add Functions by name of target object
void AStateMachineSingleton::ChangeToInterrupt(UObject* objectEntry, UObject* target, FName enterName, FName exitName, FName updateName)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{
		if (state->ExitState.IsBound())
			state->ExitState.Broadcast();

		FStateData newData = FStateData();		
		newData.EnterState = state->EnterState;
		newData.ExitState = state->ExitState;
		newData.UpdateState = state->UpdateState;
		state->stateData.Add(newData);

		state->EnterState.Clear();
		state->ExitState.Clear();
		state->UpdateState.Clear();

		FBPHelper enter = FBPHelper();
		FBPHelper exit = FBPHelper();
		FBPHelper update = FBPHelper();

		enter.BindUFunction(target, enterName);
		exit.BindUFunction(target, exitName);
		update.BindUFunction(target, updateName);

		state->EnterState.Add(enter);
		state->ExitState.Add(exit);
		state->UpdateState.Add(update);

		if (state->EnterState.IsBound())
			state->EnterState.Broadcast();
	}
}

//The specific version of Change State to call when you want the objectEntry to return to its previous state after this state is complete 
//Add Functions by delegate of target object
void AStateMachineSingleton::ChangeToInterruptByEvent(UObject* objectEntry, UObject* target, FBPHelper enter, FBPHelper exit, FBPHelper update)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{
		if (state->ExitState.IsBound())
			state->ExitState.Broadcast();

		FStateData newData = FStateData();
		newData.EnterState = state->EnterState;
		newData.ExitState = state->ExitState;
		newData.UpdateState = state->UpdateState;
		state->stateData.Add(newData);

		state->EnterState.Clear();
		state->ExitState.Clear();
		state->UpdateState.Clear();
				
		state->EnterState.Add(enter);
		state->ExitState.Add(exit);
		state->UpdateState.Add(update);

		if (state->EnterState.IsBound())
			state->EnterState.Broadcast();
	}
}

//Function to call to return to the previous state, can return to the set main state if it fails
void AStateMachineSingleton::ReturnToPreviousState(UObject* objectEntry, bool returnToMainOnFail)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{
		if (state->stateData.Num() > 0)
		{
			FStateData data = state->stateData[state->stateData.Num() - 1];

			if (state->ExitState.IsBound())
				state->ExitState.Broadcast();

			state->EnterState = data.EnterState;
			state->ExitState = data.ExitState;
			state->UpdateState = data.UpdateState;

			if (state->EnterState.IsBound())
				state->EnterState.Broadcast();
	
			if (!state->stateData.IsEmpty())
			{
				GEngine->AddOnScreenDebugMessage(0, 10.0f, FColor::Red, TEXT("State Data is not empty attempting pop"));
				state->stateData.Pop();
			}
						
			return;
		}
	}

	if (returnToMainOnFail)
	{
		ChangeToMainState(objectEntry);		
	}

}

//Set the main state of the state machine by names of functions of target object
void AStateMachineSingleton::SetMainState(UObject* objectEntry, UObject* target, FName enterName, FName exitName, FName updateName)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{
		FBPHelper enter = FBPHelper();
		FBPHelper exit = FBPHelper();
		FBPHelper update = FBPHelper();

		enter.BindUFunction(target, enterName);
		exit.BindUFunction(target, exitName);
		update.BindUFunction(target, updateName);

		state->MainEnterState.Clear();
		state->MainExitState.Clear();
		state->MainUpdateState.Clear();

		state->MainEnterState.Add(enter);
		state->MainExitState.Add(exit);
		state->MainUpdateState.Add(update);
	}

	else
	{
		FObjectState newState = FObjectState();

		FBPHelper enter = FBPHelper();
		FBPHelper exit = FBPHelper();
		FBPHelper update = FBPHelper();

		enter.BindUFunction(target, enterName);
		exit.BindUFunction(target, exitName);
		update.BindUFunction(target, updateName);

		newState.MainEnterState.Add(enter);
		newState.MainExitState.Add(exit);
		newState.MainUpdateState.Add(update);

		Objects.Add(objectEntry, newState);		
	}
}

//Set the main state of the state machine by event delegates of functions of target object
void AStateMachineSingleton::SetMainStateByEvent(UObject* objectEntry, UObject* target, FBPHelper enter, FBPHelper exit, FBPHelper update)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{		
		state->MainEnterState.Clear();
		state->MainExitState.Clear();
		state->MainUpdateState.Clear();

		state->MainEnterState.Add(enter);
		state->MainExitState.Add(exit);
		state->MainUpdateState.Add(update);
	}

	else
	{
		FObjectState newState = FObjectState();
		
		newState.MainEnterState.Add(enter);
		newState.MainExitState.Add(exit);
		newState.MainUpdateState.Add(update);

		Objects.Add(objectEntry, newState);
	}
}

//Quick method to easily change back to main state when you need to
void AStateMachineSingleton::ChangeToMainState(UObject* objectEntry)
{
	FObjectState* state = Objects.Find(objectEntry);

	if (state != nullptr)
	{
		if (state->ExitState.IsBound())
			state->ExitState.Broadcast();

		state->EnterState = state->MainEnterState;
		state->ExitState = state->MainExitState;
		state->UpdateState = state->MainUpdateState;

		if (state->EnterState.IsBound())
			state->EnterState.Broadcast();
	}
}