
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Reverse a string by bytes




``` Hack
namespace HH\Lib\Str;

function reverse(
  string $string,
): string;
```




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $string `




## Returns




- ` string `




## Examples




``` basic-usage.hack
$result = Str\reverse("example_string");
echo($result);
//result: "gnirts_elpmaxe"

$result = Str\reverse("Example_string");
echo($result);
//result:"gnirts_elpmaxE"

$result = Str\reverse("");
echo($result);
//result: ""
```
<!-- HHAPIDOC -->
