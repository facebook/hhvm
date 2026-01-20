
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the largest element of the given Traversable, or null if the
Traversable is empty




``` Hack
namespace HH\Lib\Math;

function max_by<T>(
  Traversable<T> $traversable,
  (function(T): num) $num_func,
): ?T;
```




The value for comparison is determined by the given function. In the case of
duplicate numeric keys, later values overwrite previous ones.




For numeric elements, see ` Math\max() `.




## Parameters




+ [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(T): num) $num_func `




## Returns




* ` ?T `




## Examples




```
$items = vec[tuple('foo', 9), tuple('bar', 8), tuple('baz', 7)];
$maxItem = Math\max_by($items, $item ==> $item[1]);
echo "The max item is " . $maxItem[0];
```

Ouput

```
The max item is foo
```
<!-- HHAPIDOC -->
