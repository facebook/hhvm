
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Create a temporary file using a template filename




``` Hack
namespace HH\Lib\OS;

function mkstemp(
  string $template,
): (FileDescriptor, string);
```




The template must end with exactly 6 ` X ` characters; the template may be
either a relative or absolute path, however the parent directory must already
exist.




The temporary file:

+ will be a new file (i.e. ` O_CREAT | O_EXCL `)
+ be owned by the current user
+ be created with mode 0600




## Parameters




* ` string $template `




## Returns




- ` a ` - `` FileDescriptor `` and the actual path.
<!-- HHAPIDOC -->
