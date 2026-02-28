
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict sorted by the values of the given KeyedTraversable




``` Hack
namespace HH\Lib\Dict;

function sort<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  ?(function(Tv, Tv): num) $value_comparator = NULL,
): dict<Tk, Tv>;
```




If the
optional comparator function isn't provided, the values will be sorted in
ascending order.




+ To sort by some computable property of each value, see ` Dict\sort_by() `.
+ To sort by the keys of the KeyedTraversable, see ` Dict\sort_by_key() `.




Time complexity: O((n log n) * c), where c is the complexity of the
comparator function (which is O(1) if not provided explicitly)
Space complexity: O(n)




## Parameters




* [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
* ` ?(function(Tv, Tv): num) $value_comparator = NULL `




## Returns




- ` dict<Tk, Tv> `




## Examples




``` basic-usage.hack
$result = Dict\sort(dict[1 => 2000, 2 => 1000, 3 => 9]);
print_r($result);
//result: dict[3=>9, 2=>1000, 1=>2000]

$result = Dict\sort(dict[]);
print_r($result);
//result: dict[]
```
<!-- HHAPIDOC -->
