// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#pragma once

#include "Runtime/Online/HTTP/Public/Http.h"
#include "Templates/Function.h"

#include "AlkAddBackendCaller.generated.h"

UCLASS(MinimalAPI)
class UAlkAddBackendCaller : public UObject
{
  GENERATED_BODY()
public:
  virtual void BeginDestroy() override;

  void ConfigureHostName(
    FString InHostName);

  typedef TUniqueFunction<void(FString PersistentId)> OnPersistedCallback;
  void RequestPersist(
    const FString& SetId,
    const FString& PersistentId,
    const FString& ClassName,
    const TMap<FString,FString>& NamedValues,
    OnPersistedCallback&&);

private:
  FString HostName;
  FString PersistUrl;

  TMap<FHttpRequestPtr,OnPersistedCallback> OnPersistedCallbacks;

  void OnResponsePersist(
    FHttpRequestPtr Request,
    FHttpResponsePtr Response,
    bool bWasSuccessful);
};
