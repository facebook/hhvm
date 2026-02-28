
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Create a pair of connected file descriptors




``` Hack
namespace HH\Lib\OS;

function pipe(): (FileDescriptor, FileDescriptor);
```




See ` man 2 pipe ` for details. On error, an `` ErrnoException `` will be thrown.




` O_CLOEXEC ` is implicitly set for consistent behavior between standalone CLI
mode and server modes. Use `` OS\fcntl() `` to remove if needed.




## Returns




+ ` Two ` - `` FileDescriptor ``s; the first is read-only, and the second is
  write-only. Data written to the second can be read from the first.
<!-- HHAPIDOC -->
