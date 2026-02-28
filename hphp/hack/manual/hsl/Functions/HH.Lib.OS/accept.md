
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Accept a connection on a socket




``` Hack
namespace HH\Lib\OS;

function accept(
  FileDescriptor $fd,
): (FileDescriptor, sockaddr);
```




See ` man 2 accept ` for details.




## Parameters




+ ` FileDescriptor $fd `




## Returns




* ` (FileDescriptor, sockaddr) `
<!-- HHAPIDOC -->
