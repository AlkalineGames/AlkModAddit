// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#pragma once

#include "AlkAddPersistentId.generated.h"

UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class ALKMODADDIT_API UAlkAddPersistentId : public UObject
{
  GENERATED_BODY()
public:
  UPROPERTY(BlueprintReadWrite, Category = "A L K A L I N E")
  FString Value;
};
