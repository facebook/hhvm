
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Open the specified path




``` Hack
namespace HH\Lib\OS;

function open(
  string $path,
  int $flags,
  ?int $mode = NULL,
): FileDescriptor;
```




See ` man 2 open ` for details. On error, an `` ErrnoException `` will be thrown.




## Parameters




+ ` string $path `
+ ` int $flags ` a bitmask of `` O_ `` flags; one out of ``` O_RDONLY ```, ```` O_WRONLY ````,
  and ````` O_RDWR ````` **must** be specified. `````` O_CLOEXEC `````` is implicit, so that
  standalone CLI mode is consistent with server modes. If needed, this can
  be removed with ``````` OS\fcntl() ```````.
+ ` ?int $mode = NULL ` specify the mode of the file to create if `` O_CREAT `` is specified
  and the file does not exist.




## Returns




* ` FileDescriptor `
<!-- HHAPIDOC -->
