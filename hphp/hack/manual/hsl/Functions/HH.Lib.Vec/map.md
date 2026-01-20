
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec where each value is the result of calling the given
function on the original value




``` Hack
namespace HH\Lib\Vec;

function map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2>;
```




For async functions, see ` Vec\map_async() `.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv1> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Tv1): Tv2) $value_func `




## Returns




* ` vec<Tv2> `




## Examples




``` basic-usage.hack
$numbers = vec[1, 2, 3];
$new_numbers = Vec\map($numbers, $number ==> ($number + 1));
echo "new numbers are: \n";
\print_r($new_numbers);
//Output: new numbers are: 
//vec[2, 3, 4]
```
<!-- HHAPIDOC -->
