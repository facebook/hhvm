
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns true if the given KeyedContainer contains the key




``` Hack
namespace HH\Lib\C;

function contains_key<Tk1 as arraykey, Tk2 as arraykey, Tv>(
  KeyedContainer<Tk1, Tv> $container,
  Tk2 $key,
): bool;
```




Time complexity: O(1)
Space complexity: O(1)




## Parameters




+ [` KeyedContainer<Tk1, `](/apis/Interfaces/HH/KeyedContainer/)`` Tv> $container ``
+ ` Tk2 $key `




## Returns




* ` bool `




## Examples




``` basic-usage.hack
$dict = dict["key_1" => "a", "key_2" => "b"];
$contains_key_result_1 = C\contains_key($dict, "key_1");
echo "First contains key result: $contains_key_result_1\n";
//Output: First contains key result: true

$contains_key_result_2 = C\contains_key($dict, "key_3");
echo "Second contains key result: $contains_key_result_2\n";
//Output: Second contains key result: false

$contains_key_result_3 = C\contains_key($dict, "");
echo "Third contains key result: $contains_key_result_3\n";
//Output: Third contains key result: false
```
<!-- HHAPIDOC -->
