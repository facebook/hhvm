
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where each value is the result of calling the given
function on the original value




``` Hack
namespace HH\Lib\Dict;

function map<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): dict<Tk, Tv2>;
```




To use an async function, see ` Dict\map_async() `.




Time complexity: O(n * f), where f is the complexity of ` $value_func `
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv1> $traversable ``
+ ` (function(Tv1): Tv2) $value_func `




## Returns




* ` dict<Tk, Tv2> `




## Examples




``` basic-usage.hack
$original_dict_1 = dict["key_1" => 1, "key_2" => 2, "key_3" => 3];
$dict_of_lambda_multiplied_values = Dict\map($original_dict_1, $val ==> $val * 2);
echo "Resulting lambda multiplied dict: \n";
\print_r($dict_of_lambda_multiplied_values);
//Output: Resulting lambda multiplied dict: 
//dict["key_1" => 2, "key_2" => 4, "key_3" => 6]

$original_dict_2 = dict["key_1" => "a", "key_2" => "b", "key_3" => "c"];
$dict_of_function_uppercased_values = Dict\map($original_dict_2, Str\uppercase<>);
echo "Resulting function uppercased dict: \n";
\print_r($dict_of_function_uppercased_values);
//Output: Resulting function uppercased dict:
//dict["key_1" => "A", "key_2" => "B", "key_3" => "C"]
```
<!-- HHAPIDOC -->
