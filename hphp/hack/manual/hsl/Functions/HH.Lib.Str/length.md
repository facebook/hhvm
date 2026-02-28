
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the length of the given string represented in number of bytes




``` Hack
namespace HH\Lib\Str;

function length(
  string $string,
): int;
```




This function is ` O(1) `: it always returns the number of bytes in the string,
even if a byte is null. For example, `` Str\length("foo\0bar") `` is 7, not 3.




See ` Str\length_l() ` for the length in characters.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $string `




## Returns




- ` int `




## Examples




``` basic-usage.hack
$result = Str\length("example_string");
echo($result);
//result: 14

$result = Str\length("");
echo($result);
//result: 0 
```
<!-- HHAPIDOC -->
