
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where all the given keys map to the given value




``` Hack
namespace HH\Lib\Dict;

function fill_keys<Tk as arraykey, Tv>(
  Traversable<Tk> $keys,
  Tv $value,
): dict<Tk, Tv>;
```




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tk> `](/apis/Interfaces/HH/Traversable/)`` $keys ``
+ ` Tv $value `




## Returns




* ` dict<Tk, Tv> `




## Examples




``` basic-usage.hack
$keys = vec['k1', 'k2', 'k3', 'k4'];
$value = 5;
$dict = Dict\fill_keys($keys, $value);
\print_r($dict);
```
<!-- HHAPIDOC -->
