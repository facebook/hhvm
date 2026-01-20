
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
namespace HH;

function idx_readonly<Tk as arraykey, Tv>(
  ?KeyedContainer<Tk, Tv> $collection,
  ?Tk $index,
  $default = NULL,
): Tv;
```




## Parameters




+ ` ? `[` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` Tv> $collection ``
+ ` ?Tk $index `
+ ` $default = NULL `




## Returns




* ` Tv `
<!-- HHAPIDOC -->
