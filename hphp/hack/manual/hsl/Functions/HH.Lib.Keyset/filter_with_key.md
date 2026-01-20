
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset containing only the values for which the given predicate
returns ` true `




``` Hack
namespace HH\Lib\Keyset;

function filter_with_key<Tk, Tv as arraykey>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Tk, Tv): bool) $predicate,
): keyset<Tv>;
```




If you don't need access to the key, see ` Keyset\filter() `.




Time complexity: O(n * p), where p is the complexity of ` $predicate `
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` (function(Tk, Tv): bool) $predicate `




## Returns




* ` keyset<Tv> `




## Examples




``` basic-usage.hack
$result = Keyset\filter_with_key(dict[1 => 2, 2 => 4, 3 => 5], ($key, $val) ==> $val%2==0);
print_r($result);
//result: keyset[2,4]

$result = Keyset\filter_with_key(dict[1 => 2, 2 => 4], ($key, $val) ==> $val==0);
print_r($result);
//result: keyset[]

$result = Keyset\filter_with_key(dict[1 => 2, 2 => 4, 3 => 5],  ($key, $val) ==> $val == $val);
print_r($result);
//result: keyset[2,4,5]
```
<!-- HHAPIDOC -->
