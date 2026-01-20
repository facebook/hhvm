
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
namespace HH;

function darray<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $arr,
): darray<arraykey, mixed, Tk, Tv>;
```




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $arr ``




## Returns




* ` darray<arraykey, mixed, Tk, Tv> `
<!-- HHAPIDOC -->
