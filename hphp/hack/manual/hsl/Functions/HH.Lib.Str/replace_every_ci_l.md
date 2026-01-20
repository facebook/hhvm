
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the "haystack" string with all occurrences of the keys of
` $replacements ` replaced by the corresponding values (case-insensitive)




``` Hack
namespace HH\Lib\Str;

function replace_every_ci_l(
  \HH\Lib\Locale\Locale $locale,
  string $haystack,
  KeyedContainer<string, string> $replacements,
): string;
```




Locale-specific rules for case-insensitive comparisons will be used, and
strings will be normalized before comparing if the locale specifies an
encoding that supports multiple representations of the same characters, such
as UTF-8.




Replacements are applied in the order they are specified in ` $replacements `,
and the new values are searched again for subsequent matches. For example,
`` dict['a' => 'b', 'b' => 'c'] `` is equivalent to ``` dict['a' => 'c'] ```, but
```` dict['b' => 'c', 'a' => 'b'] ```` is not, despite having the same elements.




If there are multiple overlapping matches, the match occuring earlier in
` $replacements ` (not in `` $haystack ``) takes precedence.




+ For a single case-sensitive search/replace, see ` Str\replace_l() `.
+ For a single case-insensitive search/replace, see ` Str\replace_ci_l() `.
+ For multiple case-sensitive searches/replacements, see ` Str\replace_every_l() `.
+ For not having new values searched again, see ` Str\replace_every_nonrecursive_ci_l() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` \HH\Lib\Locale\Locale $locale `
- ` string $haystack `
- [` KeyedContainer<string, `](/apis/Interfaces/HH/KeyedContainer/)`` string> $replacements ``




## Returns




+ ` string `
<!-- HHAPIDOC -->
