
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Start a socket connection




``` Hack
namespace HH\Lib\OS;

function connect(
  FileDescriptor $fd,
  sockaddr $sa,
): void;
```




See ` man 2 connect ` for details.




## Parameters




+ ` FileDescriptor $fd `
+ ` sockaddr $sa `




## Returns




* ` void `
<!-- HHAPIDOC -->
