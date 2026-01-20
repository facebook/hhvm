
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the "haystack" string with all occurrences of the keys of
` $replacements ` replaced by the corresponding values




``` Hack
namespace HH\Lib\Str;

function replace_every(
  string $haystack,
  KeyedContainer<string, string> $replacements,
): string;
```




Replacements are applied in the order they are specified in ` $replacements `,
and the new values are searched again for subsequent matches. For example,
`` dict['a' => 'b', 'b' => 'c'] `` is equivalent to ``` dict['a' => 'c'] ```, but
```` dict['b' => 'c', 'a' => 'b'] ```` is not, despite having the same elements.




If there are multiple overlapping matches, the match occuring earlier in
` $replacements ` (not in `` $haystack ``) takes precedence.




+ For a single case-sensitive search/replace, see ` Str\replace() `.
+ For a single case-insensitive search/replace, see ` Str\replace_ci() `.
+ For multiple case-insensitive searches/replacements, see ` Str\replace_every_ci() `.
+ For not having new values searched again, see ` Str\replace_every_nonrecursive() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $haystack `
- [` KeyedContainer<string, `](/apis/Interfaces/HH/KeyedContainer/)`` string> $replacements ``




## Returns




+ ` string `




## Examples




``` basic-usage.hack
$result = Str\replace_every("example_string", dict["example"=>"test", "string"=>"value"]);
echo($result);
//result: test_value

$result = Str\replace_every("example_string", dict["EXAMPLE"=>"test", "STRING"=>"value"]);
echo($result);
//result: example_string

$result = Str\replace_every("example_string", dict["uncontained"=>"test", "uncontained_2"=>"value"]);
echo($result);
//result: example_string
```
<!-- HHAPIDOC -->
