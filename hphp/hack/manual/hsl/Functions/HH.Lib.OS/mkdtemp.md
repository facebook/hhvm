
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Create a temporary directory




``` Hack
namespace HH\Lib\OS;

function mkdtemp(
  string $template,
): string;
```




This function creates a new, unique temporary directory, with the name
matching the provided template, and returns the path. The directory will be
created with permissions 0700.




The template MUST end with ` XXXXXX `; these are replaced with random
printable characters to create a unique name. While some platforms are more
flexible, the HSL always requires this for consistency. Any additional
trailing `` X ``s may result in literal X's (e.g. glibc), or in additional
randomness (e.g. BSD) - use a separator (e.g. ``` fooXXX.XXXXXX ```) to guarantee
any characters are preserved.




## Parameters




+ ` string $template `




## Returns




* ` string `
<!-- HHAPIDOC -->
