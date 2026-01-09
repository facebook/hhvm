
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Listen for new connections to a socket




``` Hack
namespace HH\Lib\OS;

function listen(
  FileDescriptor $fd,
  int $backlog,
): void;
```




See ` man 2 listen ` for details.




## Parameters




+ ` FileDescriptor $fd `
+ ` int $backlog `




## Returns




* ` void `
<!-- HHAPIDOC -->
