
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the output handle for the current request




``` Hack
namespace HH\Lib\IO;

function request_output(): WriteHandle;
```




This should generally be used for sending data to clients. In CLI mode, this
is usually the process STDOUT.




This MAY be a ` CloseableWriteFDHandle `.




## Returns




+ ` WriteHandle `
<!-- HHAPIDOC -->
