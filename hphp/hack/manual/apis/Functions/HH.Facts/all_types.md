
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return all symbols defined in the repo, as a dict mapping each symbol
name to the path where the symbol lives in the repo




``` Hack
namespace HH\Facts;

function all_types(): dict<classname<nonnull, mixed>, string>;
```




If a symbol is defined in more than one path, one of the paths defining the
symbol will be chosen in an unspecified manner.




## Returns




+ ` dict<classname<nonnull, mixed>, string> `
<!-- HHAPIDOC -->
