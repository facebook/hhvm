---
title: TClientSink
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
final class TClientSink {...}
```




### Public Methods




+ [` ->__construct(): void `](/apis/Classes/TClientSink/__construct/)
+ [` ->genCreditsOrFinalResponse(): Awaitable<?(int, ?string, ?string)> `](/apis/Classes/TClientSink/genCreditsOrFinalResponse/)
+ [` ->genCreditsOrFinalResponseHelper(): Awaitable<(int, ?string, ?Exception)> `](/apis/Classes/TClientSink/genCreditsOrFinalResponseHelper/)
+ [` ->genSink<TSinkType, TFinalResponseType>(HH\AsyncGenerator<null, TSinkType, void> $payload_generator, (function(?TSinkType, ?Exception): (string, ?bool)) $payloadEncode, (function(?string, ?Exception): TFinalResponseType) $finalResponseDecode): Awaitable<TFinalResponseType> `](/apis/Classes/TClientSink/genSink/)
+ [` ->sendClientException(string $ex_encoded_string, ?string $ex_msg): void `](/apis/Classes/TClientSink/sendClientException/)
+ [` ->sendPayloadOrSinkComplete(?string $payload): bool `](/apis/Classes/TClientSink/sendPayloadOrSinkComplete/)
<!-- HHAPIDOC -->
