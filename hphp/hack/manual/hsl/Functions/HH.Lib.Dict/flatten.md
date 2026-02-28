
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict formed by merging the KeyedContainer elements of the
given Traversable




``` Hack
namespace HH\Lib\Dict;

function flatten<Tk as arraykey, Tv>(
  Traversable<KeyedContainer<Tk, Tv>> $traversables,
): dict<Tk, Tv>;
```




In the case of duplicate keys, later values will overwrite
the previous ones.




For a fixed number of KeyedTraversables, see ` Dict\merge() `.




Time complexity: O(n), where n is the combined size of all the
` $traversables `
Space complexity: O(n), where n is the combined size of all the
`` $traversables ``




## Parameters




+ [` Traversable<KeyedContainer<Tk, `](/apis/Interfaces/HH/Traversable/)`` Tv>> $traversables ``




## Returns




* ` dict<Tk, Tv> `
<!-- HHAPIDOC -->
