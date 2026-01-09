
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Reposition the current file offset




``` Hack
namespace HH\Lib\OS;

function lseek(
  FileDescriptor $fd,
  int $offset,
  SeekWhence $whence,
): int;
```




See ` man 2 lseek ` for details. On error, an `` ErrnoException `` will be thrown.




## Parameters




+ ` FileDescriptor $fd `
+ ` int $offset `
+ ` SeekWhence $whence `




## Returns




* ` int `
<!-- HHAPIDOC -->
