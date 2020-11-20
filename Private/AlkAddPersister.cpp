// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#include "AlkAddPersister.h"

#include "AlkAddBackendCaller.h"
#include "AlkAddPersistentId.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAlkAddPersister, Log, All);
DEFINE_LOG_CATEGORY(LogAlkAddPersister);

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
  FString PersistentId = InOutPersistentId->Value;
  PersistentId.TrimStartAndEndInline();
  const UObject* Owner = Addit->GetOwner();
  FName ClassName = Owner ? Owner->GetClass()->GetFName() : "<null>";
  Impl->MutateBackendCaller().RequestPersist(
    SetId, PersistentId, ClassName.ToString(), NamedValues,
    [InOutPersistentId] (FString PersistentId) {
      InOutPersistentId->Value = PersistentId;
    }
  );
}
