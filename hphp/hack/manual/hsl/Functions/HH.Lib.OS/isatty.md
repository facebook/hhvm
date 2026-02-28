
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Test if a file descriptor refers to a terminal




``` Hack
namespace HH\Lib\OS;

function isatty(
  FileDescriptor $fd,
): bool;
```




If the native call fails with ` ENOTTY ` (for example, on MacOS), this function
will return false.




If the native call fails with any other error (for example, ` EBADF `), this
function will throw.




## Parameters




+ ` FileDescriptor $fd `




## Returns




* ` bool `
<!-- HHAPIDOC -->
