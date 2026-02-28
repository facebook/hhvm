
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing each element of the given Traversable exactly
once




``` Hack
namespace HH\Lib\Vec;

function unique<Tv as arraykey>(
  Traversable<Tv> $traversable,
): vec<Tv>;
```




The Traversable must contain arraykey values, and strict equality will
be used.




For non-arraykey elements, see ` Vec\unique_by() `.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




* ` vec<Tv> `




## Examples




``` basic-usage.hack
$vector = vec[1, 1, 2, 3, 4, 4, 4, 4];
$uniqueVals = Vec\unique($vector);
\print_r($uniqueVals);
```



<!-- HHAPIDOC -->
