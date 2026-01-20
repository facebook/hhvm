
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns true if the given `Traversable<Tv>` is sorted in ascending order




``` Hack
namespace HH\Lib\C;

function is_sorted<Tv>(
  Traversable<Tv> $traversable,
  ?(function(Tv, Tv): num) $comparator = NULL,
): bool;
```




If two neighbouring elements compare equal, this will be considered sorted.




If no $comparator is provided, the ` <=> ` operator will be used.
This will sort numbers by value, strings by alphabetical order
or by the numeric value, if the strings are well-formed numbers,
and DateTime/DateTimeImmutable by their unixtime.




To check the order of other types or mixtures of the
aforementioned types, see C\\is_sorted_by.




If the comparison operator ` <=> ` is not useful on Tv
and no $comparator is provided, the result of is_sorted
will not be useful.




Time complexity: O((n * c), where c is the complexity of the
comparator function (which is O(1) if not provided explicitly)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` ?(function(Tv, Tv): num) $comparator = NULL `




## Returns




* ` bool `




## Examples




``` basie-usage.hack
$strings = vec["a", "b", "c", "d", "e"];
$is_sorted_result_1 = C\is_sorted($strings);
echo "First is_sorted result: $is_sorted_result_1\n";
//Output: First is_sorted result: true

$second_strings = vec["a", "b", "a", "d", "e"];
$is_sorted_result_2 = C\is_sorted($second_strings);
echo "Second is_sorted result: $is_sorted_result_2\n";
//Output: Second is_sorted result: false

$empty_strings = vec[];
$is_sorted_result_3 = C\is_sorted($empty_strings);
echo "Third is_sorted result: $is_sorted_result_3\n";
//Output: Third is_sorted result: true
```
<!-- HHAPIDOC -->
