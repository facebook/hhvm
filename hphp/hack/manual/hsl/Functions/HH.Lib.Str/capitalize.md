
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the string with the first character capitalized




``` Hack
namespace HH\Lib\Str;

function capitalize(
  string $string,
): string;
```




If the first character is already capitalized or isn't alphabetic, the string
will be unchanged.




+ To capitalize all characters, see ` Str\uppercase() `.
+ To capitalize all words, see ` Str\capitalize_words() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $string `




## Returns




+ ` string `




## Examples




``` basic-usage.hack
$result = Str\capitalize("example_string");
echo($result);
// result "Example_string"

$result = Str\capitalize("Example_string");
echo($result);
// result "Example_string"
```
<!-- HHAPIDOC -->
