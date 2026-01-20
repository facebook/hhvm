
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec formed by concatenating the given Traversables together




``` Hack
namespace HH\Lib\Vec;

function concat<Tv>(
  Traversable<Tv> $first,
  Container<Tv> ...$rest,
): vec<Tv>;
```




For a variable number of Traversables, see ` Vec\flatten() `.




Time complexity: O(n + m), where n is the size of ` $first ` and m is the
combined size of all the `` ...$rest ``
Space complexity: O(n + m), where n is the size of ``` $first ``` and m is the
combined size of all the ```` ...$rest ````




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $first ``
+ [` Container<Tv> `](/apis/Interfaces/HH/Container/)`` ...$rest ``




## Returns




* ` vec<Tv> `




## Examples




``` basic-usage.hack
$original_vec = vec["abc", "def", "ghi"];
$rest = vec["xxx", "yyy"];
$concat_vec = Vec\concat($original_vec, $rest);
echo "Resulting concat vec: \n";
\print_r($concat_vec);
//Output: Resulting concat vec: 
//vec["abc", "def", "ghi", "xxx", "yyy"]
```
<!-- HHAPIDOC -->
