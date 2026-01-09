
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Clear __MemoizeLSB data

+ if $func is non-null, clear cache for $cls::$func
+ if $func is null, clear all LSB memoization caches for $cls




``` Hack
namespace HH;

function clear_lsb_memoization(
  string $cls,
  ?string $func = NULL,
): bool;
```




Operates on a single class at a time. Clearing the cache for $cls::$func
does not clear the cache for $otherClass::$func, for any other class.




## Parameters




* ` string $cls `
* ` ?string $func = NULL `




## Returns




- ` bool `
<!-- HHAPIDOC -->
