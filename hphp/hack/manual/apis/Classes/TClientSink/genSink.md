
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
public function genSink<TSinkType, TFinalResponseType>(
  HH\AsyncGenerator<null, TSinkType, void> $payload_generator,
  (function(?TSinkType, ?Exception): (string, ?bool)) $payloadEncode,
  (function(?string, ?Exception): TFinalResponseType) $finalResponseDecode,
): Awaitable<TFinalResponseType>;
```




## Parameters




+ [` HH\AsyncGenerator<null, `](/apis/Classes/HH/AsyncGenerator/)`` TSinkType, void> $payload_generator ``
+ ` (function(?TSinkType, ?Exception): (string, ?bool)) $payloadEncode `
+ ` (function(?string, ?Exception): TFinalResponseType) $finalResponseDecode `




## Returns




* [` Awaitable<TFinalResponseType> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
