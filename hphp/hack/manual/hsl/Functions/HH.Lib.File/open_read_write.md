
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
namespace HH\Lib\File;

function open_read_write(
  string $path,
  WriteMode $mode = WriteMode::OPEN_OR_CREATE,
  int $create_file_permissions = 420,
): CloseableReadWriteHandle;
```




## Parameters




+ ` string $path `
+ ` WriteMode $mode = WriteMode::OPEN_OR_CREATE `
+ ` int $create_file_permissions = 420 `




## Returns




* ` CloseableReadWriteHandle `
<!-- HHAPIDOC -->
