// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#include "AlkAddPersister.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAlkAddPersister, Log, All);
DEFINE_LOG_CATEGORY(LogAlkAddPersister);

// static
FString // returns PersistentId
UAlkAddPersister::AlkAddPersist(
  FString InSetId, // blank uses "<company name>:<project name>"
  FString InPersistentId, // blank generates new ID, i.e. creation
  const UActorComponent* Addit,
  const TMap<FString,FString>& NamedValues)
{
  FString SetId = InSetId;
  SetId.TrimStartAndEndInline();
  if (SetId.IsEmpty())
  {
    FString CompanyName;
    GConfig->GetString(
      TEXT("/Script/EngineSettings.GeneralProjectSettings"),
      TEXT("CompanyName"),
      CompanyName,
      GGameIni
    );
    FString ProjectName;
    GConfig->GetString(
      TEXT("/Script/EngineSettings.GeneralProjectSettings"),
      TEXT("ProjectName"),
      ProjectName,
      GGameIni
    );
    SetId = CompanyName + ':' + ProjectName;
  }
  FString PersistentId = InPersistentId;
  PersistentId.TrimStartAndEndInline();
  if (PersistentId.IsEmpty())
  {
    PersistentId = "###PlaceholderPersistentId###";
  }
  // TODO: ### COMPLETE IMPLEMENTATION
  return PersistentId;
}
