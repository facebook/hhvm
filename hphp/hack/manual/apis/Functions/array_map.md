
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` array_map ` previously had it's signature rewritten based on the arity of
the call, to match runtime behaviors including:

+ Preserving the input container type
+ Allowing for ` N ` args and accepting a function of the same arity




``` Hack
function array_map<Tk as arraykey, Tin, Tout>(
  (function(Tin): Tout) $callback,
  KeyedContainer<Tk, Tin> $arr,
): KeyedContainer<Tk, Tout>;
```




This runtime behavior still exists but this function is deprecated in favor
of HSL functions like ` Vec\map ` or `` Dict\map ``.




## Parameters




* ` (function(Tin): Tout) $callback `
* [` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` Tin> $arr ``




## Returns




- [` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` Tout> ``
<!-- HHAPIDOC -->
