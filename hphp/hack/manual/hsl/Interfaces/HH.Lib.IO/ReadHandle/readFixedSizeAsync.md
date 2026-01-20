
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Read a fixed amount of data




``` Hack
public function readFixedSizeAsync(
  int $size,
  ?int $timeout_ns = NULL,
): Awaitable<string>;
```




Will fail with ` EPIPE ` if the file is closed before that much data is
available.




It is possible for this to never return, e.g. if called on a pipe or
or socket which the other end keeps open forever. Set a timeout if you
do not want this to happen.




## Parameters




+ ` int $size `
+ ` ?int $timeout_ns = NULL `




## Returns




* [` Awaitable<string> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
