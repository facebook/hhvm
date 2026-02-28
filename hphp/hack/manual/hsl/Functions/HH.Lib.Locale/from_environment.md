
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retrieve the active locale from the native environment




``` Hack
namespace HH\Lib\Locale;

function from_environment(): Locale;
```




This is usually set based on the ` LC_* ` environment variables.




Web applications targeting diverse users should probably not use this,
however it is useful when aiming to support diverse users in CLI
programs.




## Returns




+ ` Locale `
<!-- HHAPIDOC -->
