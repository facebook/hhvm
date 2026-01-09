
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Attach metadata to the caller's stack frame




``` Hack
namespace HH;

function set_frame_metadata(
  mixed $metadata,
): void;
```




The metadata can be retrieved
using debug_backtrace(DEBUG_BACKTRACE_PROVIDE_METADATA).




## Parameters




+ ` mixed $metadata `




## Returns




* ` void `
<!-- HHAPIDOC -->
