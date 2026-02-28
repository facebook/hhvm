
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

function filter<Tv as arraykey>(
  Traversable<Tv> $traversable,
  ?(function(Tv): bool) $value_predicate = NULL,
): keyset<Tv>;
```




The default predicate is casting the value to boolean.




To remove null values in a typechecker-visible way, see ` Keyset\filter_nulls() `.




Time complexity: O(n * p), where p is the complexity of ` $value_predicate `
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` ?(function(Tv): bool) $value_predicate = NULL `




## Returns




* ` keyset<Tv> `




## Examples




``` basic-usage.hack
$result = Keyset\filter(keyset[1,2,3,4], $val ==> $val%2==0);
print_r($result);
//result: keyset[2,4]

$result = Keyset\filter(keyset[1,2,3,4], $val ==> $val==0);
print_r($result);
//result: keyset[]

$result = Keyset\filter(keyset[1,2,3,4],  $val ==> $val == $val);
print_r($result);
//result: keyset[1,2,3,4]
```
<!-- HHAPIDOC -->
