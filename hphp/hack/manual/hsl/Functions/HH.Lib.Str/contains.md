
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the "haystack" string contains the "needle" string




``` Hack
namespace HH\Lib\Str;

function contains(
  string $haystack,
  string $needle,
  int $offset = 0,
): bool;
```




An optional offset determines where in the haystack the search begins. If the
offset is negative, the search will begin that many characters from the end
of the string. If the offset is out-of-bounds, a ViolationException will be
thrown.




+ To get the position of the needle, see ` Str\search() `.
+ To search for the needle case-insensitively, see ` Str\contains_ci() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $haystack `
- ` string $needle `
- ` int $offset = 0 `




## Returns




+ ` bool `




## Examples




``` basic-usage.hack
$result = Str\contains("example_string", "example");
echo($result);
//result: true 

$result = Str\contains("example_string", "EXAMPLE"); // different case
echo($result);
//result: false

$result = Str\contains("example_string", "example", 2); // with offset
echo($result);
//result: false 

$result = Str\contains("example_string", "uncontained");
echo($result);
//result: false 
```
<!-- HHAPIDOC -->
