
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

function replace_every_nonrecursive_l(
  \HH\Lib\Locale\Locale $locale,
  string $haystack,
  KeyedContainer<string, string> $replacements,
): string;
```




Once a substring has
been replaced, its new value will not be searched again.




Strings will be normalized for comparison in encodings that support multiple
representations, such as UTF-8.




If there are multiple overlapping matches, the match occuring earlier in
` $haystack ` takes precedence. If a replacer is a prefix of another (like
"car" and "carpet"), the longer one (carpet) takes precedence. The ordering
of `` $replacements `` therefore doesn't matter.




+ For having new values searched again, see ` Str\replace_every_l() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` \HH\Lib\Locale\Locale $locale `
- ` string $haystack `
- [` KeyedContainer<string, `](/apis/Interfaces/HH/KeyedContainer/)`` string> $replacements ``




## Returns




+ ` string `
<!-- HHAPIDOC -->
