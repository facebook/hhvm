
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first key of the given KeyedTraversable, or throws if the
KeyedTraversable is empty




``` Hack
namespace HH\Lib\C;

function first_keyx<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
): Tk;
```




For possibly empty Traversables, see ` C\first_key `.




Time complexity: O(1)
Space complexity: O(1)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``




## Returns




* ` Tk `




## Examples




``` basic-usage.hack
$dict = dict["key_1" => "a", "key_2" => "b", "key_3" => "c"];
$first_keyx_result_1 = C\first_keyx($dict);
echo "First first keyx result: $first_keyx_result_1\n";
//Output: First first keyx result: key_1

$empty_dict = dict[];
$first_keyx_result_2 = C\first_keyx($empty_dict);
//Output: Hit a php exception : exception 'InvariantViolationException' with message 
//'HH\Lib\C\first_keyx: Expected at least one element.' 
```
<!-- HHAPIDOC -->
