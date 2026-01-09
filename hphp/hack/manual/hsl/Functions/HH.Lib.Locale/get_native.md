
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retrieve the locale being used by libc functions for the current thread




``` Hack
namespace HH\Lib\Locale;

function get_native(): Locale;
```




In general, we discourage this: it can be surprising that it changes the
behavior of many libc functions, like ` sprintf('%f' `), and error messages
from native code may be translated.




For web applications, that's likely unwanted - we recommend frameworks add
the concept of a 'viewer locale', and explicitly pass it to the relevant
string functions instead.




## Returns




+ ` Locale `
<!-- HHAPIDOC -->
