
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict keyed by the result of calling the given function on each
corresponding value




``` Hack
namespace HH\Lib\Dict;

function from_values<Tk as arraykey, Tv>(
  Traversable<Tv> $values,
  (function(Tv): Tk) $key_func,
): dict<Tk, Tv>;
```




In the case of duplicate keys, later values will
overwrite the previous ones.




+ To create a dict from keys, see ` Dict\from_keys() `.
+ To create a dict from key/value tuples, see ` Dict\from_entries() `.
+ To create a dict containing all values with the same keys, see ` Dict\group_by() `.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




* [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $values ``
* ` (function(Tv): Tk) $key_func `




## Returns




- ` dict<Tk, Tv> `




## Examples




``` hack.basic-usage.hack
$original_dict_1 = dict[1 => 1, 2 => 2, 3 => 3];
$from_values_dict_1 = Dict\from_values($original_dict_1, $x ==> $x + 1);
echo "Resulting from values dict 1: \n";
\print_r($from_values_dict_1);
//Output: Resulting from values dict 1:
//dict[2 => 1, 3 => 2, 4 => 3]

$original_dict_2 = dict[1 => 1, 2 => 2, 3 => 3];
$from_values_repeat_value_dict_2 = Dict\from_values($original_dict_2, $x ==> $x * 0);
echo "Resulting from repeat values dict 2: \n";
\print_r($from_values_repeat_value_dict_2);
//Output: Resulting from repeat values dict 2: 
//dict[0 => 3]
```
<!-- HHAPIDOC -->
