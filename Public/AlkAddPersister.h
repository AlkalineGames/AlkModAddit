// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#pragma once

#include "AlkAddPersister.generated.h"

UCLASS()
class UAlkAddPersister : public UObject
{
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category = "A L K A L I N E")
  static FString AlkAddPersist( // returns PersitentId
      FString SetId, // must be non-blank
      FString PersistentId, // blank generates new ID, i.e. creation
      const UActorComponent* Addit,
      const TMap<FString,FString>& NamedValues
  );
};
