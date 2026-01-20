---
title: TClientBufferedStream
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
final class TClientBufferedStream {...}
```




### Public Methods




+ [` ->__construct(): void `](/apis/Classes/TClientBufferedStream/__construct/)
+ [` ->gen<TStreamResponse>((function(?string, ?Exception): TStreamResponse) $streamDecode): HH\AsyncGenerator<null, TStreamResponse, void> `](/apis/Classes/TClientBufferedStream/gen/)
+ [` ->genNext(): Awaitable<(?vec<string>, ?string)> `](/apis/Classes/TClientBufferedStream/genNext/)
<!-- HHAPIDOC -->
