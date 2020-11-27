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
    SetId, [WorldContextObject] (const TArray<UAlkAddBackendCaller::FPersistentObjectState>& PersistentObjectStateArray)
    {
      for (const auto& PersistentObjectState : PersistentObjectStateArray)
      {
        UClass* Class = FindObject<UClass>(
          ANY_PACKAGE, *PersistentObjectState.ClassName);
        if (Class)
        {
          FTransform Transform; // ### TODO: GET FROM PERSISTENT STATE
          FActorSpawnParameters ActorSpawnParameters;
          ActorSpawnParameters.SpawnCollisionHandlingOverride =
            //ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
          WorldContextObject->GetWorld()->SpawnActor(
            //Class, &Transform, ActorSpawnParameters);
            Class, NULL, NULL, ActorSpawnParameters);
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
  if (!InOutPersistentId)
  {
    UE_LOG(LogAlkAddPersister, Error, TEXT("nullptr passed for PersistentId"));
    return;
  }
  if (!Addit)
  {
    UE_LOG(LogAlkAddPersister, Error, TEXT("nullptr passed for Addit"));
    return;
  }
  FString SetId = ResolvedSetId(InSetId);
  FString PersistentId = InOutPersistentId->Value;
  PersistentId.TrimStartAndEndInline();
  const UObject* Owner = Addit->GetOwner();
  FString ClassName = Owner ? Owner->GetClass()->GetFName().ToString() : TEXT("<null>");
  Impl->MutateBackendCaller().RequestPersistCreate(
    {SetId, PersistentId, ClassName, NamedValues},
    [InOutPersistentId] (FString PersistentId) {
      InOutPersistentId->Value = PersistentId;
    }
  );
}

