
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Renders a Regex Pattern to a string




``` Hack
namespace HH\Lib\Regex;

function to_string(
  Pattern<Match> $pattern,
): string;
```




The regex pattern follows the PCRE library: https://www.pcre.org/original/doc/html/pcresyntax.html.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` Pattern<Match> $pattern `




## Returns




- ` string `
<!-- HHAPIDOC -->
