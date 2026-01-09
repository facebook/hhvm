
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the error output handle for the current request




``` Hack
namespace HH\Lib\IO;

function request_errorx(): CloseableWriteFDHandle;
```




This is usually only available for CLI scripts; it will fail with `EBADF.
in most other contexts, including HTTP requests.




For a non-throwing version, use ` request_error() `.




In CLI mode, this is usually the process STDERR.




## Returns




+ ` CloseableWriteFDHandle `
<!-- HHAPIDOC -->
