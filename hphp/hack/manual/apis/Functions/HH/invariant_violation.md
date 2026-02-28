
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Call this when one of your ` invariant `s has been violated




``` Hack
namespace HH;

function invariant_violation(
  FormatString<\PlainSprintf> $format_str,
  ...$fmt_args,
): noreturn;
```




It calls the
function you registered with ` invariant_callback_register ` and then throws
an [` InvariantException `](/apis/Classes/HH/InvariantException/)




## Parameters




+ ` FormatString<\PlainSprintf> $format_str ` - The string that will be displayed when your invariant
  fails.
+ ` ...$fmt_args `




## Returns




* ` noreturn `
<!-- HHAPIDOC -->
