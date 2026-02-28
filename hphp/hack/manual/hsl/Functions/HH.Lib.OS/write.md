
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Write from the specified ` FileDescriptor `




``` Hack
namespace HH\Lib\OS;

function write(
  FileDescriptor $fd,
  string $data,
): int;
```




See ` man 2 write ` for details. On error, an `` ErrnoException `` will be thrown.




## Parameters




+ ` FileDescriptor $fd `
+ ` string $data `




## Returns




* ` the ` - number of bytes written; it is possible for this function to
  succeed with a partial write.
<!-- HHAPIDOC -->
