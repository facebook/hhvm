
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates a new temporary file, without automatic cleanup




``` Hack
namespace HH\Lib\File;

function leaky_temporary_file(
  string $prefix = 'hack-leakytmp-',
  string $suffix = '',
): CloseableReadWriteHandle;
```




` File\temporary_file() ` is **strongly** recommended instead.




+ If the prefix starts with ` . `, it is interpreted relative to the current
  working directory.
+ If the prefix statis with ` / `, it is treated as an absolute path.
+ Otherwise, it is created in the system temporary directory.




Regardless of the kind of prefix, the parent directory must exist.




A suffix can optionally be provided; this is useful when you need a
particular filename extension; for example,
` File\temporary_file('foo', '.txt') ` may create `` /tmp/foo123456.txt ``.




The temporary file:

* will be a new file (i.e. ` O_CREAT | O_EXCL `)
* be owned by the current user
* be created with mode 0600
* **will not** be automatically deleted




## Parameters




- ` string $prefix = 'hack-leakytmp-' `
- ` string $suffix = '' `




## Returns




+ ` CloseableReadWriteHandle `
<!-- HHAPIDOC -->
