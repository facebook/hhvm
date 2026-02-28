
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Create a temporary file using a template filename, with an invariant suffix,
and the specified open flags




``` Hack
namespace HH\Lib\OS;

function mkostemps(
  string $template,
  int $suffix_length,
  int $flags,
): (FileDescriptor, string);
```




The template must contain exactly 6 ` X ` characters, followed immediately
by the invariant suffix.




The length of the suffix must be specified; for example, if the template is
` fooXXXXXXbar `, the suffix len is 3, or for `` fooXXXXXXbarbaz `` it is 6. For
``` fooXXXXXXXXXXXXXXXXXX ```, any suffix len between 0 and 12 is valid.




The template may be either a relative or absolute path, however the parent
directory must already exist.




This function takes the same flags as ` OS\open() `; like that function,
`` O_CLOEXEC `` is implied.




The temporary file:

+ will be a new file (i.e. ` O_CREAT | O_EXCL `)
+ be owned by the current user
+ be created with mode 0600




## Parameters




* ` string $template `
* ` int $suffix_length `
* ` int $flags `




## Returns




- ` a ` - `` FileDescriptor `` and the actual path.
<!-- HHAPIDOC -->
