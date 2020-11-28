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

void UAlkAddBackendCaller::RequestPersistCreate(
  const FPersistentObjectStateRefs& PersistentObjectState,
  FOnResponsePersistCreateCallback&& InOnResponsePersisteCreateCallback)
{
  FString Payload;
  TSharedRef<TJsonWriter<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>>
    JsonWriter = TJsonWriterFactory<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>::Create(&Payload);
  JsonWriter->WriteObjectStart();
  JsonWriter->WriteValue(TEXT("set_id"), PersistentObjectState.SetId);
  JsonWriter->WriteValue(TEXT("persistent_id"), PersistentObjectState.PersistentId);
  JsonWriter->WriteValue(TEXT("class_name"), PersistentObjectState.ClassName);
  JsonWriter->WriteValue(TEXT("transform"), PersistentObjectState.Transform.ToString());
  for (auto& NamedValue : PersistentObjectState.NamedValues)
    JsonWriter->WriteValue(NamedValue.Key, NamedValue.Value);
  JsonWriter->WriteObjectEnd();
  JsonWriter->Close();

  auto Request = FHttpModule::Get().CreateRequest();
  OnResponsePersistCreateCallbacks.Add(Request,
    MoveTemp(InOnResponsePersisteCreateCallback));
  Request->OnProcessRequestComplete().BindUObject(
    this, &UAlkAddBackendCaller::OnResponsePersistCreate);
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

void UAlkAddBackendCaller::RequestPersistRetrieve(
  const FString& SetId,
  FOnResponsePersistRetrieveCallback&& InOnResponsePersisteRetrieveCallback)
{
  auto Request = FHttpModule::Get().CreateRequest();
  OnResponsePersistRetrieveCallbacks.Add(Request,
    MoveTemp(InOnResponsePersisteRetrieveCallback));
  Request->OnProcessRequestComplete().BindUObject(
    this, &UAlkAddBackendCaller::OnResponsePersistRetrieve);
  Request->SetVerb(TEXT("GET"));
  Request->SetURL(PersistUrl + "/" + SetId);
  Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
  Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
  Request->ProcessRequest();
  UE_LOG(LogAlkAddBackendCaller, Warning,
    TEXT("%s %s"), *(Request->GetVerb()), *PersistUrl);
  // !!! necessary to force a response if we are being called
  // !!! before game is running and ticks are not yet firing
  FHttpModule::Get().GetHttpManager().Flush(false);
}

// private methods

void UAlkAddBackendCaller::OnResponsePersistCreate(
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
  auto Callback = OnResponsePersistCreateCallbacks.Find(Request);
  if (Callback) {
    (*Callback)(PersistentId);
    OnResponsePersistCreateCallbacks.Remove(Request);
  }
}

void UAlkAddBackendCaller::OnResponsePersistRetrieve(
  FHttpRequestPtr Request,
  FHttpResponsePtr Response,
  bool bWasSuccessful
) {
  UE_LOG(LogAlkAddBackendCaller, Warning,
    TEXT("%s %s: RC %i, content <%s>"),
    *(Request->GetVerb()), *PersistUrl,
    Response->GetResponseCode(),
    *(Response->GetContentAsString()));
  TArray<FPersistentObjectState> PersistentObjectStateArray;
  if (bWasSuccessful)
  {
    TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Response->GetContentAsString());
    TArray<TSharedPtr<FJsonValue>> JsonArray;
    if (FJsonSerializer::Deserialize(Reader, JsonArray))
    {
      for (auto& JsonValue : JsonArray)
      {
        const TSharedPtr<FJsonObject>* JsonObject = nullptr;
        if (JsonValue->TryGetObject(JsonObject) && JsonObject)
        {
          auto& Item = PersistentObjectStateArray.AddDefaulted_GetRef();
          for (auto& MapEntry : (*JsonObject)->Values)
          {
            FString ValueString;
            MapEntry.Value->TryGetString(ValueString);
            if (MapEntry.Key == "set_id")
              Item.SetId = ValueString;
            else if (MapEntry.Key == "persistent_id")
              Item.PersistentId = ValueString;
            else if (MapEntry.Key == "class_name")
              Item.ClassName = ValueString;
            else if (MapEntry.Key == "transform")
              Item.Transform.InitFromString(ValueString);
            else
              Item.NamedValues.Add(MapEntry.Key, ValueString);
          }
        }
      }
    }
  }
  auto Callback = OnResponsePersistRetrieveCallbacks.Find(Request);
  if (Callback) {
    (*Callback)(PersistentObjectStateArray);
    OnResponsePersistRetrieveCallbacks.Remove(Request);
  }
}
