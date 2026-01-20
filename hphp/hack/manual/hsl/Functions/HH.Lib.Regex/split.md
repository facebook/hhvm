
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Splits ` $haystack ` into chunks by its substrings that match with `` $pattern ``




``` Hack
namespace HH\Lib\Regex;

function split(
  string $haystack,
  Pattern<Match> $delimiter,
  ?int $limit = NULL,
): vec<string>;
```




If ` $limit ` is given, the returned vec will have at most that many elements.
The last element of the vec will be whatever is left of the haystack string
after the appropriate number of splits.
If no substrings of `` $haystack `` match ``` $delimiter ```, a vec containing only ```` $haystack ```` will be returned.
The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.




Throws [` InvariantException `](/apis/Classes/HH/InvariantException/) if `` $limit `` < 2.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $haystack `
* ` Pattern<Match> $delimiter `
* ` ?int $limit = NULL `




## Returns




- ` vec<string> `
<!-- HHAPIDOC -->
