
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns ` $haystack ` with any substring matching `` $pattern ``
replaced by ``` $replacement ```




``` Hack
namespace HH\Lib\Regex;

function replace(
  string $haystack,
  Pattern<Match> $pattern,
  string $replacement,
  int $offset = 0,
): string;
```




If ` $offset ` is given, replacements are made
only starting from `` $offset ``.
The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.




Throws [` InvariantException `](/apis/Classes/HH/InvariantException/) if `` $offset `` is not within plus/minus the length of ``` $haystack ```.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $haystack `
* ` Pattern<Match> $pattern `
* ` string $replacement `
* ` int $offset = 0 `




## Returns




- ` string `
<!-- HHAPIDOC -->
