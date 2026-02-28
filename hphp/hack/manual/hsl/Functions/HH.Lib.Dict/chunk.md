
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a vec containing the original dict split into chunks of the given
size




``` Hack
namespace HH\Lib\Dict;

function chunk<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  int $size,
): vec<dict<Tk, Tv>>;
```




If the original dict doesn't divide evenly, the final chunk will be
smaller.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` int $size `




## Returns




* ` vec<dict<Tk, Tv>> `




## Examples




``` basic-usage.hack
$result = Dict\chunk(dict[1 => 2, 2 => 4], 1);
print_r($result);
// result: vec[dict[1=>2], dict[2=>4]]

$result = Dict\chunk(dict[], 1);
print_r($result);
//result: vec[]
```
<!-- HHAPIDOC -->
