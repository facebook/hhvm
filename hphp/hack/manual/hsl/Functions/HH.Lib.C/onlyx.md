
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first and only element of the given Traversable, or throws if the
Traversable is empty or contains more than one element




``` Hack
namespace HH\Lib\C;

function onlyx<T>(
  Traversable<T> $traversable,
  ?\HH\Lib\Str\SprintfFormatString $format_string = NULL,
  mixed ...$format_args,
): T;
```




An optional format string (and format arguments) may be passed to specify
a custom message for the exception in the error case.




For Traversables with more than one element, see ` C\firstx `.




Time complexity: O(1)
Space complexity: O(1)




## Parameters




+ [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` ?\HH\Lib\Str\SprintfFormatString $format_string = NULL `
+ ` mixed ...$format_args `




## Returns




* ` T `




## Examples




``` basic-usage.hack
$string = vec["a"];
$onlyx_result_1 = C\onlyx($string);
echo "First onlyx result: $onlyx_result_1\n";
//Output: First onlyx result: a

$strings = vec["a", "b"];
$onlyx_result_2 = C\onlyx($strings);
//Output: Hit a php exception : exception 'InvariantViolationException' with message
//'Expected exactly one element but got 2.'

$empty_strings = vec[];
$onlyx_result_3 = C\onlyx($empty_strings);
//Output: Hit a php exception : exception 'InvariantViolationException' with message
//'Expected non-empty Traversable.'
```
<!-- HHAPIDOC -->
