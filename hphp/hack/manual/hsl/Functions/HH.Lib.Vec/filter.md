
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing only the values for which the given predicate
returns ` true `




``` Hack
namespace HH\Lib\Vec;

function filter<Tv>(
  Traversable<Tv> $traversable,
  ?(function(Tv): bool) $value_predicate = NULL,
): vec<Tv>;
```




The default predicate is casting the value to boolean.




+ To remove null values in a typechecker-visible way, see
  ` Vec\filter_nulls() `.
+ To use an async predicate, see ` Vec\filter_async() `.




Time complexity: O(n * p), where p is the complexity of ` $value_predicate `
Space complexity: O(n)




## Parameters




* [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
* ` ?(function(Tv): bool) $value_predicate = NULL `




## Returns




- ` vec<Tv> `




## Examples




``` basic-usage.hack
$numbers = vec[1, 2, 3, 4, 5];
$filter_numbers = Vec\filter($numbers, $number ==> $number % 2 === 0);
echo "Filter numbers are: \n";
\print_r($filter_numbers);
//Output: Filter numbers are:
//vec[2, 4]
```
<!-- HHAPIDOC -->
