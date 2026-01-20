
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Read until there is no more data to read




``` Hack
public function readAllAsync(
  ?int $max_bytes = NULL,
  ?int $timeout_ns = NULL,
): Awaitable<string>;
```




It is possible for this to never return, e.g. if called on a pipe or
or socket which the other end keeps open forever. Set a timeout if you
do not want this to happen.




Up to ` $max_bytes ` may be allocated in a buffer; large values may lead to
unnecessarily hitting the request memory limit.




## Parameters




+ ` ?int $max_bytes = NULL `
+ ` ?int $timeout_ns = NULL `




## Returns




* [` Awaitable<string> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
