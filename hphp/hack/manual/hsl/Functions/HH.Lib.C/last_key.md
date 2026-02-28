
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last key of the given KeyedTraversable, or null if the
KeyedTraversable is empty




``` Hack
namespace HH\Lib\C;

function last_key<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
): ?Tk;
```




For non-empty Traversables, see ` C\last_keyx `.




Time complexity: O(1) if ` $traversable ` is a [` Container `](/apis/Interfaces/HH/Container/), O(n) otherwise.
Space complexity: O(1)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``




## Returns




* ` ?Tk `




## Examples




``` basic-usage.hack
$dict = dict["key_1" => "a", "key_2" => "b", "key_3" => "c"];
$last_key_result_1 = C\last_key($dict);
echo "First last key result: $last_key_result_1\n";
//Output: First last key result: key_3

$empty_dict = dict[];
$last_key_result_2 = C\last_key($empty_dict);
$last_key_result_2_as_string = $last_key_result_2 ?? "null";
echo "Second last key result: $last_key_result_2_as_string\n";
//Output: Second last key result: null
```
<!-- HHAPIDOC -->
