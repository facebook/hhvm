
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the input handle for the current request




``` Hack
namespace HH\Lib\IO;

function request_input(): ReadHandle;
```




In CLI mode, this is likely STDIN; for HTTP requests, it may contain the
POST data, if any.




This MAY be a ` CloseableReadFDHandle `.




## Returns




+ ` ReadHandle `
<!-- HHAPIDOC -->
