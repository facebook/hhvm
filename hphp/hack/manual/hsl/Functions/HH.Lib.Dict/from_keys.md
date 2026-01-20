
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where each value is the result of calling the given
function on the corresponding key




``` Hack
namespace HH\Lib\Dict;

function from_keys<Tk as arraykey, Tv>(
  Traversable<Tk> $keys,
  (function(Tk): Tv) $value_func,
): dict<Tk, Tv>;
```




+ To use an async function, see ` Dict\from_key_async() `.
+ To create a dict from values, see ` Dict\from_values() `.
+ To create a dict from key/value tuples, see ` Dict\from_entries() `.




Time complexity: O(n * f), where f is the complexity of ` $value_func `
Space complexity: O(n)




## Parameters




* [` Traversable<Tk> `](/apis/Interfaces/HH/Traversable/)`` $keys ``
* ` (function(Tk): Tv) $value_func `




## Returns




- ` dict<Tk, Tv> `




## Examples




``` basic-usage.hack
$keys = vec[1, 2, 3, 4];
$dict = Dict\from_keys($keys, $x ==> $x + 1);
\print_r($dict);

$values = vec[1, 2, 3, 4];
$dict = Dict\from_keys($keys, $x ==> 5);
\print_r($dict);
```
<!-- HHAPIDOC -->
