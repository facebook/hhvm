
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a substring of length ` $length ` of the given string starting at the
`` $offset ``




``` Hack
namespace HH\Lib\Str;

function slice(
  string $string,
  int $offset,
  ?int $length = NULL,
): string;
```




If no length is given, the slice will contain the rest of the
string. If the length is zero, the empty string will be returned. If the
offset is out-of-bounds, a ViolationException will be thrown.




Previously known as ` substr ` in PHP.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $string `
* ` int $offset `
* ` ?int $length = NULL `




## Returns




- ` string `




## Examples




``` basic-usage.hack
$result = Str\slice("example_string", 5);
echo($result);
//result: "le_string"

$result = Str\slice("example_string", 5, 1);
echo($result);
//result: "l"

$result = Str\slice("example_string", 2, 0);
echo($result);
//result: ""

$result = Str\slice("example_string", 1000);
echo($result);
//result: <Throws exception>
```
<!-- HHAPIDOC -->
