
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing the subsequence of the given Traversable
determined by the offset and length




``` Hack
namespace HH\Lib\Vec;

function slice<Tv>(
  Container<Tv> $container,
  int $offset,
  ?int $length = NULL,
): vec<Tv>;
```




If no length is given or it exceeds the upper bound of the Traversable,
the vec will contain every element after the offset.




+ To take only the first ` $n ` elements, see `` Vec\take() ``.
+ To drop the first ` $n ` elements, see `` Vec\drop() ``.




Time complexity: O(n), where n is the size of the slice
Space complexity: O(n), where n is the size of the slice




## Parameters




* [` Container<Tv> `](/apis/Interfaces/HH/Container/)`` $container ``
* ` int $offset `
* ` ?int $length = NULL `




## Returns




- ` vec<Tv> `




## Examples




``` basic-usage.hack
$vector = vec[1, 2, 3, 4, 5, 6, 7];
$sliced_vector = Vec\slice($vector, 3);
\print_r($sliced_vector);

$sliced_vector2 = Vec\slice($vector, 2, 3);
\print_r($sliced_vector2);
```
<!-- HHAPIDOC -->
