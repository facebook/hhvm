
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing only the elements of the first Traversable
that do not appear in the second one, where an element's identity is
determined by the scalar function




``` Hack
namespace HH\Lib\Vec;

function diff_by<Tv, Ts as arraykey>(
  Traversable<Tv> $first,
  Traversable<Tv> $second,
  (function(Tv): Ts) $scalar_func,
): vec<Tv>;
```




For vecs that contain arraykey elements, see ` Vec\diff() `.




Time complexity: O((n + m) * s), where n is the size of ` $first `, m is the
size of `` $second ``, and s is the complexity of ``` $scalar_func ```
Space complexity: O(n + m), where n is the size of ```` $first ```` and m is the size
of ````` $second ````` -- note that this is bigger than O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $first ``
+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $second ``
+ ` (function(Tv): Ts) $scalar_func `




## Returns




* ` vec<Tv> `
<!-- HHAPIDOC -->
