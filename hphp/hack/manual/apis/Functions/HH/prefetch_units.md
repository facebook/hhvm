
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Prefetch units corresponding to the given sets of paths, before
they are actually needed




``` Hack
namespace HH;

function prefetch_units(
  keyset<string> $paths,
  bool $hint,
): void;
```




+ If $hint is true, this request is non-binding. That is, the
  runtime may attempt to asynchronously prefetch the units in the
  background, or maybe not do anything at all. The units may not
  (and probably won't be) loaded when this function returns.

+ If $hint is false, the runtime will always attempt to prefetch
  the units and will block until they have been. Therefore when
  this function returns, the units have been loaded. Note:
  depending on configuration, this may be slower than a non-binding
  request.





## Parameters




* ` keyset<string> $paths `
* ` bool $hint `




## Returns




- ` void `
<!-- HHAPIDOC -->
