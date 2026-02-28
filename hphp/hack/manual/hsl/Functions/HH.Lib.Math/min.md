
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the smallest element of the given Traversable, or null if the
Traversable is empty




``` Hack
namespace HH\Lib\Math;

function min<T as num>(
  Traversable<T> $numbers,
): ?T;
```




+ For a known number of inputs, see ` Math\minva() `.
+ To find the largest number, see ` Math\max() `.




## Parameters




* [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $numbers ``




## Returns




- ` ?T `




## Examples




``` basic-usage.hack
$v = vec[3, 2, 6, 1, 4];
$min = Math\min($v);
echo "The min is $min";
```
<!-- HHAPIDOC -->
