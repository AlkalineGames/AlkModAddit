// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

#include "AlkAddBackendCaller.h"

#include "Runtime/Online/HTTP/Public/HttpManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAlkAddBackendCaller, Log, All);
DEFINE_LOG_CATEGORY(LogAlkAddBackendCaller);

// virtual
void UAlkAddBackendCaller::BeginDestroy() // override
{
  Super::BeginDestroy();
}

void UAlkAddBackendCaller::ConfigureHostName(
  FString InHostName)
{
  HostName = InHostName;
  PersistUrl = "http://" + HostName + "/session/persist";
}

void UAlkAddBackendCaller::RequestPersist(
  const FString& SetId,
  const FString& PersistentId,
  const FString& ClassName,
  const TMap<FString,FString>& NamedValues,
  OnPersistedCallback&& InOnPersisted)
{
  OnPersisted = MoveTemp(InOnPersisted);
  FString Payload;
  TSharedRef<TJsonWriter<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>>
    JsonWriter = TJsonWriterFactory<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>::Create(&Payload);
  JsonWriter->WriteObjectStart();
  JsonWriter->WriteValue(TEXT("set_id"), SetId);
  JsonWriter->WriteValue(TEXT("persistent_id"), PersistentId);
  JsonWriter->WriteValue(TEXT("class_name"), ClassName);
  for (auto& NamedValue : NamedValues)
    JsonWriter->WriteValue(NamedValue.Key, NamedValue.Value);
  JsonWriter->WriteObjectEnd();
  JsonWriter->Close();

  auto Request = FHttpModule::Get().CreateRequest();
  Request->OnProcessRequestComplete().BindUObject(
    this, &UAlkAddBackendCaller::OnResponsePersist);
  Request->SetVerb(TEXT("POST"));
  Request->SetURL(PersistUrl);
  Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
  Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
  Request->SetContentAsString(Payload);
  Request->ProcessRequest();
  UE_LOG(LogAlkAddBackendCaller, Warning,
    TEXT("%s %s: content <%s>"),
    *(Request->GetVerb()), *PersistUrl, *Payload);
  // !!! necessary to force a response if we are being called
  // !!! before game is running and ticks are not yet firing
  FHttpModule::Get().GetHttpManager().Flush(false);
}

// private methods

void UAlkAddBackendCaller::OnResponsePersist(
  FHttpRequestPtr Request,
  FHttpResponsePtr Response,
  bool bWasSuccessful
) {
  UE_LOG(LogAlkAddBackendCaller, Warning,
    TEXT("%s %s: RC %i, content <%s>"),
    *(Request->GetVerb()), *PersistUrl,
    Response->GetResponseCode(),
    *(Response->GetContentAsString()));
  FString PersistentId = "<null>";
  if (bWasSuccessful) {
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (FJsonSerializer::Deserialize(Reader, JsonObject))
      PersistentId = JsonObject->GetStringField(TEXT("persistent_id"));
  }
  OnPersisted(PersistentId);
}
