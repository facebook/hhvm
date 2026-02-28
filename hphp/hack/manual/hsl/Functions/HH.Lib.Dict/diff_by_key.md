
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict containing only the entries of the first KeyedTraversable
whose keys do not appear in any of the other ones




``` Hack
namespace HH\Lib\Dict;

function diff_by_key<Tk1 as arraykey, Tk2 as arraykey, Tv>(
  KeyedTraversable<Tk1, Tv> $first,
  KeyedTraversable<Tk2, mixed> $second,
  KeyedContainer<Tk2, mixed> ...$rest,
): dict<Tk1, Tv>;
```




Time complexity: O(n + m), where n is size of ` $first ` and m is the combined
size of `` $second `` plus all the ``` ...$rest ```
Space complexity: O(n + m), where n is size of ```` $first ```` and m is the combined
size of ````` $second ````` plus all the `````` ...$rest `````` -- note that this is bigger than
O(n)




## Parameters




+ [` KeyedTraversable<Tk1, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $first ``
+ [` KeyedTraversable<Tk2, `](/apis/Interfaces/HH/KeyedTraversable/)`` mixed> $second ``
+ [` KeyedContainer<Tk2, `](/apis/Interfaces/HH/KeyedContainer/)`` mixed> ...$rest ``




## Returns




* ` dict<Tk1, Tv> `
<!-- HHAPIDOC -->
