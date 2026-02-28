
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Close the specified ` FileDescriptor `




``` Hack
namespace HH\Lib\OS;

function close(
  FileDescriptor $fd,
): void;
```




See ` man 2 close ` for details. On error, an `` ErrnoException `` will be thrown.




This function is not automatically retried on ` EINTR `, as `` close() `` is not
safe to retry on ``` EINTR ```.




## Parameters




+ ` FileDescriptor $fd `




## Returns




* ` void `
<!-- HHAPIDOC -->
