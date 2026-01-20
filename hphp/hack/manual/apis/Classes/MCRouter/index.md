---
title: MCRouter
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
class MCRouter {...}
```




### Public Methods




+ [` ::createSimple(Vector $servers): MCRouter `](/apis/Classes/MCRouter/createSimple/)\
  Simplified constructor

+ [` ::getOpName(int $op): string `](/apis/Classes/MCRouter/getOpName/)\
  Translate an mc_op_* numeric code to something human-readable

+ [` ::getResultName(int $op): string `](/apis/Classes/MCRouter/getResultName/)\
  Translate an mc_res_* numeric code to something human-readable

+ [` ->__construct(darray<string, mixed> $options, string $pid = ''): void `](/apis/Classes/MCRouter/__construct/)\
  Initialize an MCRouter handle

+ [` ->add(string $key, string $value, int $flags = 0, int $expiration = 0): Awaitable<void> `](/apis/Classes/MCRouter/add/)\
  Store a value

+ [` ->append(string $key, string $value): Awaitable<void> `](/apis/Classes/MCRouter/append/)\
  Modify a value

+ [` ->cas(int $cas, string $key, string $value, int $expiration = 0): Awaitable<void> `](/apis/Classes/MCRouter/cas/)\
  Compare and set

+ [` ->decr(string $key, int $val): Awaitable<int> `](/apis/Classes/MCRouter/decr/)\
  Atomicly decrement a numeric value

+ [` ->del(string $key): Awaitable<void> `](/apis/Classes/MCRouter/del/)\
  Delete a key

+ [` ->flushAll(int $delay = 0): Awaitable<void> `](/apis/Classes/MCRouter/flushAll/)\
  Flush all key/value pairs

+ [` ->get(string $key): Awaitable<string> `](/apis/Classes/MCRouter/get/)\
  Retreive a value

+ [` ->gets(string $key): Awaitable<shape('value' => string, 'cas' => int, 'flags' => int), darray> `](/apis/Classes/MCRouter/gets/)\
  Retreive a record and its metadata

+ [` ->incr(string $key, int $val): Awaitable<int> `](/apis/Classes/MCRouter/incr/)\
  Atomicly increment a numeric value

+ [` ->prepend(string $key, string $value): Awaitable<void> `](/apis/Classes/MCRouter/prepend/)\
  Modify a value

+ [` ->replace(string $key, string $value, int $flags = 0, int $expiration = 0): Awaitable<void> `](/apis/Classes/MCRouter/replace/)\
  Store a value

+ [` ->set(string $key, string $value, int $flags = 0, int $expiration = 0): Awaitable<void> `](/apis/Classes/MCRouter/set/)\
  Store a value (replacing if present)

+ [` ->version(): Awaitable<string> `](/apis/Classes/MCRouter/version/)\
  Get the remote server's current version




<!-- HHAPIDOC -->
