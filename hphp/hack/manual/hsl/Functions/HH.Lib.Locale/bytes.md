
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retrieve a fixed locale suitable for byte-based operations




``` Hack
namespace HH\Lib\Locale;

function bytes(): Locale;
```




This is similar to the "C" locale, also known as the "POSIX" or "en_US_POSIX"
locale; it does not vary based on user/environment/machine settings.




It differs from the real "C" locale in that it is usable on strings that
contain null bytes; for example, ` Str\length_l(Locale\bytes(), "foo\0bar") `
will return 7, instead of 3. The behavior is equivalent if the strings
are well-formed.




## Returns




+ ` Locale `
<!-- HHAPIDOC -->
