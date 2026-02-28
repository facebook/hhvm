
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first match found in ` $haystack ` given the regex pattern `` $pattern ``
and an optional offset at which to start the search




``` Hack
namespace HH\Lib\Regex;

function first_match<T as Match>(
  string $haystack,
  Pattern<T> $pattern,
  int $offset = 0,
): ?T;
```




The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.




Throws [` InvariantException `](/apis/Classes/HH/InvariantException/) if `` $offset `` is not within plus/minus the length of ``` $haystack ```.
Returns null if there is no match, or a Match containing

+ the entire matching string, at key 0,
+ the results of unnamed capture groups, at integer keys corresponding to
  the groups' occurrence within the pattern, and
+ the results of named capture groups, at string keys matching their respective names.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $haystack `
- ` Pattern<T> $pattern `
- ` int $offset = 0 `




## Returns




+ ` ?T `
<!-- HHAPIDOC -->
