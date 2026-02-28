
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Control operations for file descriptors




``` Hack
namespace HH\Lib\OS;

function fcntl(
  FileDescriptor $fd,
  FcntlOp $cmd,
  ?int $arg = NULL,
): mixed;
```




See ` man 2 fcntl ` for details. On error, an `` ErrnoException `` will be thrown.




## Parameters




+ ` FileDescriptor $fd `
+ ` FcntlOp $cmd `
+ ` ?int $arg = NULL `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
