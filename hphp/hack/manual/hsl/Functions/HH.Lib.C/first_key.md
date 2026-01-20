
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first key of the given KeyedTraversable, or null if the
KeyedTraversable is empty




``` Hack
namespace HH\Lib\C;

function first_key<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
): ?Tk;
```




For non-empty Traversables, see ` C\first_keyx `.




Time complexity: O(1)
Space complexity: O(1)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``




## Returns




* ` ?Tk `




## Examples




``` basic-usage.hack
$dict = dict["key_1" => "a", "key_2" => "b", "key_3" => "c"];
$first_key_result_1 = C\first_key($dict);
echo "First first key result: $first_key_result_1\n";
//Output: First first key result: key_1

$empty_dict = dict[];
$first_key_result_2 = C\first_key($empty_dict);
$first_key_result_2_as_string = $first_key_result_2 ?? "null";
echo "Second first key result: $first_key_result_2_as_string\n";
//Output: Second first key result: null
```
<!-- HHAPIDOC -->
