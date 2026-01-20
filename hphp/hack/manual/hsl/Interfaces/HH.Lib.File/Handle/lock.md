
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get a shared or exclusive lock on the file




``` Hack
public function lock(
  HH\Lib\File\LockType $type,
): HH\Lib\File\Lock;
```




This will block until it acquires the lock, which may be forever.




This involves a blocking syscall; async code will not execute while
waiting for a lock.




## Parameters




+ ` HH\Lib\File\LockType $type `




## Returns




* [` HH\Lib\File\Lock `](/hsl/Classes/HH.Lib.File/Lock/)
<!-- HHAPIDOC -->
