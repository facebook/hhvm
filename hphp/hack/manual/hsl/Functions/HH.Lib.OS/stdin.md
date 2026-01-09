
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get a file descriptor for request STDIN




``` Hack
namespace HH\Lib\OS;

function stdin(): FileDescriptor;
```




Fails with EBADF if a request-specific file descriptor is not available, for
example, when running in HTTP server mode.




## Returns




+ ` FileDescriptor `
<!-- HHAPIDOC -->
