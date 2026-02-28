
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a string formed by joining the elements of the Traversable with the
given ` $glue ` string




``` Hack
namespace HH\Lib\Str;

function join(
  Traversable<arraykey> $pieces,
  string $glue,
): string;
```




Previously known as ` implode ` in PHP.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* [` Traversable<arraykey> `](/apis/Interfaces/HH/Traversable/)`` $pieces ``
* ` string $glue `




## Returns




- ` string `




## Examples




``` basic-usage.hack
$strings= vec["a", "b", "c", "d"];

$joined_string_1 = Str\join($strings, ",");
echo "First string: $joined_string_1 \n";

$joined_string_2 = Str\join($strings, "-");
echo "Second string: $joined_string_2";
```
<!-- HHAPIDOC -->
