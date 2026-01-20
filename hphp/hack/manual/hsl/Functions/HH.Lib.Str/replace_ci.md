
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the "haystack" string with all occurrences of ` $needle ` replaced by
`` $replacement `` (case-insensitive)




``` Hack
namespace HH\Lib\Str;

function replace_ci(
  string $haystack,
  string $needle,
  string $replacement,
): string;
```




+ For a case-sensitive search/replace, see ` Str\replace() `.
+ For multiple case-sensitive searches/replacements, see ` Str\replace_every() `.
+ For multiple case-insensitive searches/replacements, see ` Str\replace_every_ci() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $haystack `
- ` string $needle `
- ` string $replacement `




## Returns




+ ` string `




## Examples




``` basic-usage.hack
$result = Str\replace_ci("example_string", "string", "replacement");
echo($result);
//result: example_replacement

$result = Str\replace_ci("example_string", "STRING", "replacement");
echo($result);
//result: example_replacement

$result = Str\replace_ci("example_string", "uncontained", "replacement");
echo($result);
//result: example_string
```
<!-- HHAPIDOC -->
