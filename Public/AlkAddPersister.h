// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#pragma once

#include "AlkAddPersister.generated.h"

class UAlkAddAcoAdditBase;

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
    const FString& HostName);

  UFUNCTION(BlueprintCallable, Category = "A L K A L I N E")
  static UClass* AlkFindClassByName(
    const FString& ClassName);

  UFUNCTION(BlueprintCallable, Category = "A L K A L I N E")
  static UObject* AlkFindObjectByNames(
    const FString& ClassName,
    const FString& ObjectName);

  UFUNCTION(BlueprintCallable, Category = "A L K A L I N E", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
  void AlkAddLoadAll(
    const FString& SetId, // blank uses "<company name>:<project name>"
    const UObject* WorldContextObject
  );

  UFUNCTION(BlueprintCallable, Category = "A L K A L I N E")
  void AlkAddPersist(
    const FString& SetId, // blank uses "<company name>:<project name>"
    UAlkAddAcoAdditBase* Addit // blank PersistentId is replaced with a new value, i.e. creation
  );

  UFUNCTION(BlueprintCallable, Category = "A L K A L I N E")
  void AlkAddUnpersist(
    const FString& SetId, // blank uses "<company name>:<project name>"
    const FString& PersistentId // blank unpersists the entire set
  );
};
