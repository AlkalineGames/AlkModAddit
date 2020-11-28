// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#include "AlkAddPersister.h"

#include "AlkAddBackendCaller.h"
#include "AlkAddPersistentId.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAlkAddPersister, Log, All);
DEFINE_LOG_CATEGORY(LogAlkAddPersister);

static FString ResolvedSetId(
  const FString& InSetId) // blank uses "<company name>:<project name>"
{
  FString SetId = InSetId;
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
    SetId = CompanyName + "-" + ProjectName;
	  SetId = SetId.Replace(TEXT(" "), TEXT("_")); // convert spaces to underscores
  }
  SetId.TrimStartAndEndInline();
  return SetId;
}

class UAlkAddPersister::PrivateImpl
{
  TWeakObjectPtr<UAlkAddBackendCaller> BackendCaller;
public:
  FString HostName;
  bool IsSpawning = false;

  UAlkAddBackendCaller& MutateBackendCaller()
  {
    if (!BackendCaller.IsValid())
    {
      BackendCaller = NewObject<UAlkAddBackendCaller>();
      BackendCaller->AddToRoot(); // !!! prevent premature GC
      BackendCaller->ConfigureHostName(HostName);
    }
    return *BackendCaller;
  }
  ~PrivateImpl()
  {
    if (BackendCaller.IsValid())
      BackendCaller->RemoveFromRoot(); // !!! allow GC
  }
};

// private
UAlkAddPersister::UAlkAddPersister(
  const class FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer),
    Impl(new PrivateImpl)
{}

// virtual
void UAlkAddPersister::BeginDestroy() // override
{
  delete Impl;
  Impl = nullptr;
  Super::BeginDestroy();
}

// static
UAlkAddPersister*
UAlkAddPersister::AlkAddCreatePersister(
  FString HostName)
{
  auto Object = NewObject<UAlkAddPersister>();
  Object->Impl->HostName = HostName;
  return Object;
}

void
UAlkAddPersister::AlkAddLoadAll(
  const FString& InSetId, // blank uses "<company name>:<project name>"
  const UObject* WorldContextObject)
{
  if (!WorldContextObject)
  {
    UE_LOG(LogAlkAddPersister, Error, TEXT("null WorldContextObject"));
    return;
  }
  if (!WorldContextObject->GetWorld())
  {
    UE_LOG(LogAlkAddPersister, Error, TEXT("null World in WorldContextObject"));
    return;
  }
  FString SetId = ResolvedSetId(InSetId);
  Impl->MutateBackendCaller().RequestPersistRetrieve(
    SetId, [this,WorldContextObject] (const TArray<UAlkAddBackendCaller::FPersistentObjectState>& PersistentObjectStateArray)
    {
      for (const auto& PersistentObjectState : PersistentObjectStateArray)
      {
        UClass* Class = FindObject<UClass>(
          ANY_PACKAGE, *PersistentObjectState.ClassName);
        if (Class)
        {
          FActorSpawnParameters ActorSpawnParameters;
          ActorSpawnParameters.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            //ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
          this->Impl->IsSpawning = true; // TODO: @@@ HACKISH
          WorldContextObject->GetWorld()->SpawnActor(
            Class, &PersistentObjectState.Transform,
            ActorSpawnParameters);
            // ^ will log its own errors
          this->Impl->IsSpawning = false; // TODO: @@@ HACKISH
        }
        else
          UE_LOG(LogAlkAddPersister, Error,
            TEXT("did not find class name <%s>"),
            *PersistentObjectState.ClassName);
      }
    }
  );
}

void
UAlkAddPersister::AlkAddPersist(
  const FString& InSetId, // blank uses "<company name>:<project name>"
  UAlkAddPersistentId* InOutPersistentId,
   // ^ blank generates a new ID, i.e. creation, so this reference
   // is held until its value is assigned at asynchronous completion
  const UActorComponent* Addit,
  const TMap<FString,FString>& NamedValues)
{
  if (Impl->IsSpawning) { // TODO: @@@ HACKISH
    return; // !!! prevent reflective persistence caused by spawning
  }
  if (!InOutPersistentId)
  {
    UE_LOG(LogAlkAddPersister, Error, TEXT("null passed for PersistentId"));
    return;
  }
  if (!Addit)
  {
    UE_LOG(LogAlkAddPersister, Error, TEXT("null passed for Addit"));
    return;
  }
  if (!Addit->GetOwner())
  {
    UE_LOG(LogAlkAddPersister, Error, TEXT("null owner of Addit"));
    return;
  }
  const AActor* Actor = Cast<AActor>(Addit->GetOwner());
  if (!Actor)
  {
    UE_LOG(LogAlkAddPersister, Error, TEXT("owner is not an Actor class"));
    return;
  }
  Impl->MutateBackendCaller().RequestPersistCreate(
    {
      ResolvedSetId(InSetId),
      InOutPersistentId->Value.TrimStartAndEnd(),
      Actor->GetClass()->GetFName().ToString(),
      Actor->GetTransform(),
      NamedValues
    },
    [InOutPersistentId] (FString PersistentId) {
      InOutPersistentId->Value = PersistentId;
    }
  );
}

