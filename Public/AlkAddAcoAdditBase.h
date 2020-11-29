// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"

#include "AlkAddAcoAdditBase.generated.h"

UCLASS(Blueprintable)
class ALKMODADDIT_API UAlkAddAcoAdditBase : public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	FString PersistentId;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString,FString> PersistentNamedValues;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void EstablishAfterActorBeginPlay();

	UFUNCTION(BlueprintImplementableEvent)
	void ReadPersistentState();

	UFUNCTION(BlueprintImplementableEvent)
	void WritePersistentState(); // !!! cannot be const because it is overriden to call event dispatcher
};
