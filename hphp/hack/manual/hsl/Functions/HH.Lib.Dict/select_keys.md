
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict containing only the keys found in both the input container
and the given Traversable




``` Hack
namespace HH\Lib\Dict;

function select_keys<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $container,
  Traversable<Tk> $keys,
): dict<Tk, Tv>;
```




The dict will have the same ordering as the
` $keys ` Traversable.




Time complexity: O(k), where k is the size of ` $keys `.
Space complexity: O(k), where k is the size of `` $keys ``.




## Parameters




+ [` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` Tv> $container ``
+ [` Traversable<Tk> `](/apis/Interfaces/HH/Traversable/)`` $keys ``




## Returns




* ` dict<Tk, Tv> `




## Examples




``` basic-usage.hack
$dict_with_all_keys = dict["key1" => "value 1", "key2" => "value 2", "key3" => "value 3"];

$present_required_keys = vec["key1"];
$keys_not_present_in_dict = vec["incorrect_keys"];

$dict_with_present_keys = Dict\select_keys($dict_with_all_keys, $present_required_keys);
echo"Result when keys present in dict: \n";
\print_r($dict_with_present_keys);

$dict_with_keys_not_present = Dict\select_keys($dict_with_all_keys, $keys_not_present_in_dict);
echo"Result when keys not present in dict: \n";
\print_r($dict_with_keys_not_present);
```
<!-- HHAPIDOC -->
