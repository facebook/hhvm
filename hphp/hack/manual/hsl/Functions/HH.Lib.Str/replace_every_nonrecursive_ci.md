
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

function replace_every_nonrecursive_ci(
  string $haystack,
  KeyedContainer<string, string> $replacements,
): string;
```




Once a substring has been replaced, its new value will not be searched
again.




If there are multiple overlapping matches, the match occuring earlier in
` $haystack ` takes precedence. If a replacer is a case-insensitive prefix of
another (like "Car" and "CARPET"), the longer one (carpet) takes precedence.
The ordering of `` $replacements `` therefore doesn't matter.




When two replacers are passed that are identical except for case,
an InvalidArgumentException is thrown.




Time complexity: O(a + length * b), where a is the sum of all key lengths and
b is the sum of distinct key lengths (length is the length of ` $haystack `)




+ For having new values searched again, see ` Str\replace_every_ci() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $haystack `
- [` KeyedContainer<string, `](/apis/Interfaces/HH/KeyedContainer/)`` string> $replacements ``




## Returns




+ ` string `
<!-- HHAPIDOC -->
