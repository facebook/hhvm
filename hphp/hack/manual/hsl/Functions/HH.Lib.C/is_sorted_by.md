
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns true if the given `Traversable<Tv>` would be sorted in ascending order
after having been ` Vec\map `ed with $scalar_func sorted in ascending order




``` Hack
namespace HH\Lib\C;

function is_sorted_by<Tv, Ts>(
  Traversable<Tv> $traversable,
  (function(Tv): Ts) $scalar_func,
  ?(function(Ts, Ts): num) $comparator = NULL,
): bool;
```




If two neighbouring elements compare equal, this will be considered sorted.




If no $comparator is provided, the ` <=> ` operator will be used.
This will sort numbers by value, strings by alphabetical order
or by the numeric value, if the strings are well-formed numbers,
and DateTime/DateTimeImmutable by their unixtime.




To check the order without a mapping function,
see ` C\is_sorted `.




If the comparison operator ` <=> ` is not useful on Ts
and no $comparator is provided, the result of is_sorted_by
will not be useful.




Time complexity: O((n * c), where c is the complexity of the
comparator function (which is O(1) if not provided explicitly)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Tv): Ts) $scalar_func `
+ ` ?(function(Ts, Ts): num) $comparator = NULL `




## Returns




* ` bool `




## Examples




``` basie-usage.hack
$strings = vec[1,2,3,4,5];
$is_sorted_by_result_1 = C\is_sorted_by($strings, $x ==> $x);
echo "First is_sorted_by result: $is_sorted_by_result_1\n";
//Output: First is_sorted_by result: true

$is_sorted_by_result_2 = C\is_sorted_by($strings, $x ==> -$x);
echo "Second is_sorted_by result: $is_sorted_by_result_2\n";
//Output: Second is_sorted_by result: false
```
<!-- HHAPIDOC -->
