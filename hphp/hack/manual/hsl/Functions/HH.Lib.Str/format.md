
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Given a valid format string (defined by ` SprintfFormatString `), return a
formatted string using `` $format_args ``







``` Hack
namespace HH\Lib\Str;

function format(
  SprintfFormatString $format_string,
  mixed ...$format_args,
): string;
```




## Guides




+ [String](</hack/built-in-types/string>)
+ [Format Strings](</hack/functions/format-strings>)







## Parameters




* ` SprintfFormatString $format_string `
* ` mixed ...$format_args `




## Returns




- ` string `
<!-- HHAPIDOC -->
