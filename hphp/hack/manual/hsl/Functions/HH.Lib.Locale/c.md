
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

:::warning
**Deprecated**: Use ` Locale\bytes() ` instead.
:::




``` Hack
namespace HH\Lib\Locale;

function c(): Locale;
```




This function is being removed as:

+ there is often confusion between "the C locale" and
  "the current libc locale"
+ HHVM implements optimizations which are a visible behavior change; for
  example, ` strlen("foo\0bar") ` is 7 in HHVM, but 3 in libc.




## Returns




* ` Locale `
<!-- HHAPIDOC -->
