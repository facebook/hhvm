
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a vec containing the string split into chunks of the given size




``` Hack
namespace HH\Lib\Str;

function chunk(
  string $string,
  int $chunk_size = 1,
): vec<string>;
```




To split the string on a delimiter, see ` Str\split() `.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $string `
* ` int $chunk_size = 1 `




## Returns




- ` vec<string> `




## Examples




``` basic-usage.hack
$result = Str\chunk("example_string", 5);
print_r($result);
//result: ["examp", "le_st", "ring"]

$result = Str\chunk("example_string", 50);
print_r($result);
//result: ["example_string"]
```
<!-- HHAPIDOC -->
