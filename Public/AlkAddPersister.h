// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#pragma once

#include "AlkAddPersister.generated.h"

class UAlkAddPersistentId;

UCLASS(BlueprintType, Transient)
class ALKMODADDIT_API UAlkAddPersister : public UObject
{
  class PrivateImpl;
  PrivateImpl* Impl = nullptr;
  GENERATED_BODY()
  UAlkAddPersister(const class FObjectInitializer& ObjectInitializer);
public:
  virtual void BeginDestroy() override;

  UFUNCTION(BlueprintCallable, Category = "A L K A L I N E")
  static UAlkAddPersister* AlkAddCreatePersister(
    FString HostName);

  UFUNCTION(BlueprintCallable, Category = "A L K A L I N E")
  void AlkAddPersist(
    const FString& SetId, // blank uses "<company name>:<project name>"
    UAlkAddPersistentId* PersistentId,
     // ^ blank generates a new ID, i.e. creation, so this reference
     // is held until its value is assigned at asynchronous completion
    const UActorComponent* Addit,
    const TMap<FString,FString>& NamedValues
  );
};
