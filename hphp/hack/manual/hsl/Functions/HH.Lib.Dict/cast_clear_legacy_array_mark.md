
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Casts the given traversable to a dict, resetting the legacy array mark
if applicable




``` Hack
namespace HH\Lib\Dict;

function cast_clear_legacy_array_mark<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $x,
): dict<Tk, Tv>;
```




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $x ``




## Returns




* ` dict<Tk, Tv> `
<!-- HHAPIDOC -->
