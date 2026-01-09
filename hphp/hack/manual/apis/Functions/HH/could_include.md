
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the (php) file could be included (eg if its been compiled
into the binary)




``` Hack
namespace HH;

function could_include(
  string $file,
): bool;
```




This is useful when you don't have a filesystem
(RepoAuthoritative mode) but still want to know if including a file will
work.




## Parameters




+ ` string $file `




## Returns




* ` bool `
<!-- HHAPIDOC -->
