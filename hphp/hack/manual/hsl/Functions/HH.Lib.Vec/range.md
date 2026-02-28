
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing the range of numbers from ` $start ` to `` $end ``
inclusive, with the step between elements being ``` $step ``` if provided, or 1 by
default




``` Hack
namespace HH\Lib\Vec;

function range<Tv as num>(
  Tv $start,
  Tv $end,
  ?Tv $step = NULL,
): vec<Tv>;
```




If ` $start > $end `, it returns a descending range instead of
an empty one.




If you don't need the items to be enumerated, consider Vec\\fill.




Time complexity: O(n), where ` n ` is the size of the resulting vec
Space complexity: O(n), where `` n `` is the size of the resulting vec




## Parameters




+ ` Tv $start `
+ ` Tv $end `
+ ` ?Tv $step = NULL `




## Returns




* ` vec<Tv> `




## Examples




``` basic-usage.hack
$result = Vec\range(1, 10, 1);
print_r($result);
//result: [1,2,3,4,5,6,7,8,9,10]

$result = Vec\range(1, 10, 2);
print_r($result);
//result: [1,3,5,7,9]

$result = Vec\range(1, 10, 3);
print_r($result);
//result: [1,4,7,10]
```
<!-- HHAPIDOC -->
