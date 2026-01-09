
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get the types defined in the given path




``` Hack
namespace HH;

function autoload_path_to_types(
  string $path,
): vec<classname<mixed>>;
```




The path may be relative to the repo root or absolute. But this function
will not dereference symlinks for you, so providing a path with symlinks
may cause this function to return an empty vec when you expected results.




Throws InvalidOperationException if native autoloading is disabled.




## Parameters




+ ` string $path `




## Returns




* ` vec<classname<mixed>> `
<!-- HHAPIDOC -->
