
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset containing all of the elements of the given
Traversables




``` Hack
namespace HH\Lib\Keyset;

function union<Tv as arraykey>(
  Traversable<Tv> $first,
  Container<Tv> ...$rest,
): keyset<Tv>;
```




For a variable number of Traversables, see ` Keyset\flatten() `.




Time complexity: O(n + m), where n is the size of ` $first ` and m is the
combined size of all the `` ...$rest ``
Space complexity: O(n + m), where n is the size of ``` $first ``` and m is the
combined size of all the ```` ...$rest ````




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $first ``
+ [` Container<Tv> `](/apis/Interfaces/HH/Container/)`` ...$rest ``




## Returns




* ` keyset<Tv> `




## Examples




``` basic-usage.hack
$result = Keyset\union(keyset[1,2], vec[4,5] );
print_r($result);
//result: keyset[1,2,4,5]

$result = Keyset\union(keyset[1,2], vec[3,3,4,5] );
print_r($result);
//result: keyset[1,2,3,4,5]
```
<!-- HHAPIDOC -->
