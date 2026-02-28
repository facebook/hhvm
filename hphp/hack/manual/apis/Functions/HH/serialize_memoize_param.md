
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Takes an argument to a function marked with `<<__Memoize>>` and serializes it
to a string usable as a unique cache key




``` Hack
namespace HH;

function serialize_memoize_param(
  mixed $param,
): arraykey;
```




This works with all builtin types
and with objects that implement the HH\\IMemoizeParam interface




## Parameters




+ ` mixed $param `




## Returns




* ` arraykey `
<!-- HHAPIDOC -->
