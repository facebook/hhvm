
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get address of the connected peer




``` Hack
namespace HH\Lib\OS;

function getpeername(
  FileDescriptor $fd,
): sockaddr;
```




See ` man 2 getpeername ` for details.




## Parameters




+ ` FileDescriptor $fd `




## Returns




* ` sockaddr `
<!-- HHAPIDOC -->
