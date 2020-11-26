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

  typedef TUniqueFunction<void(FString PersistentId)> FOnResponsePersistCreateCallback;
  void RequestPersistCreate(
    const FString& SetId,
    const FString& PersistentId,
    const FString& ClassName,
    const TMap<FString,FString>& NamedValues,
    FOnResponsePersistCreateCallback&&);

  struct FPersistentObjectState {
    FString ClassName;
    TMap<FString,FString> NamedValues;
  };
  typedef TUniqueFunction<void(const TArray<FPersistentObjectState>&)> FOnResponsePersistRetrieveCallback;
  void RequestPersistRetrieve(
    const FString& SetId,
    FOnResponsePersistRetrieveCallback&&);

private:
  FString HostName;
  FString PersistUrl;

  TMap<FHttpRequestPtr,FOnResponsePersistCreateCallback> OnResponsePersistCreateCallbacks;
  void OnResponsePersistCreate(
    FHttpRequestPtr Request,
    FHttpResponsePtr Response,
    bool bWasSuccessful);

  TMap<FHttpRequestPtr,FOnResponsePersistRetrieveCallback> OnResponsePersistRetrieveCallbacks;
  void OnResponsePersistRetrieve(
    FHttpRequestPtr Request,
    FHttpResponsePtr Response,
    bool bWasSuccessful);
};
