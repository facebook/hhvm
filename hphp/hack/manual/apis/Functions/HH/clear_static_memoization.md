
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Clear memoization data

+ if $cls is non-null, clear memoziation cache for $cls::$func,
  or for all static memoized methods if $func is null
+ if $cls is null, clear memoization cache for $func




``` Hack
namespace HH;

function clear_static_memoization(
  ?string $cls,
  ?string $func = NULL,
): bool;
```




## Parameters




* ` ?string $cls `
* ` ?string $func = NULL `




## Returns




- ` bool `
<!-- HHAPIDOC -->
