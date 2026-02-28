
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns ` < 0 ` if `` $string1 `` is less than ``` $string2 ```, ```` > 0 ```` if ````` $string1 ````` is
greater than `````` $string2 ``````, and ``````` 0 ``````` if they are equal (case-insensitive)




``` Hack
namespace HH\Lib\Str;

function compare_ci(
  string $string1,
  string $string2,
): int;
```




For a case-sensitive comparison, see ` Str\compare() `.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $string1 `
* ` string $string2 `




## Returns




- ` int `




## Examples




``` basic-usage.hack
$comparison_1 = Str\compare_ci("apple", "banana");
echo "Result of first comparison: $comparison_1 \n";

$comparison_2 = Str\compare_ci("banana", "apple");
echo "Result of second comparison: $comparison_2 \n";

$comparison_3 = Str\compare_ci("apple", "Banana");
echo "Result of third comparison: $comparison_3 \n";

$comparison_4 = Str\compare_ci("apple", "Apple");
echo "Result of fourth comparison: $comparison_4 \n";
```
<!-- HHAPIDOC -->
