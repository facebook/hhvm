
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return all paths currently known to the autoloader




``` Hack
namespace HH;

function autoload_get_paths(): Container<string>;
```




This may or may not be all the paths in your repo. If you call
` HH\autoload_set_paths() ` with a callback and expect that callback to
lazily load paths as it sees new symbols, this function will only return
all paths which we have seen during this request.




If native autoloading is enabled, or if every path passed to
` HH\autoload_set_paths() ` was a valid path with all symlinks dereferenced,
then each path returned will be an absolute canonical path, with all
symlinks dereferenced.




Throws InvalidOperationException if autoloading is disabled.




## Returns




+ [` Container<string> `](/apis/Interfaces/HH/Container/)
<!-- HHAPIDOC -->
