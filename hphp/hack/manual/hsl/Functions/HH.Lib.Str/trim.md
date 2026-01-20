
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the given string with whitespace stripped from the beginning and end




``` Hack
namespace HH\Lib\Str;

function trim(
  string $string,
  ?string $char_mask = NULL,
): string;
```




If the optional character mask isn't provided, the following characters will
be stripped: space, tab, newline, carriage return, NUL byte, vertical tab.




+ To only strip from the left, see ` Str\trim_left() `.
+ To only strip from the right, see ` Str\trim_right() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $string `
- ` ?string $char_mask = NULL `




## Returns




+ ` string `




## Examples




``` basic-usage.hack
$result = Str\trim("example_string    ");
echo($result);
//result: "example_string"

$result = Str\trim("   example_string  ");
echo($result);
//result: "example_string"
```
<!-- HHAPIDOC -->
