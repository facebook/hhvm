
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Displays fatal errors with this PHP document




``` Hack
function hphp_set_error_page(
  string $page,
): void;
```




When 500 fatal error is about to display, it will invoke this PHP page with
all global states right at when the error happens. This is useful for
gracefully displaying something helpful information to end users when a fatal
error has happened. Otherwise, a blank page will be displayed by default.




## Parameters




+ ` string $page ` - Relative path of the PHP document.




## Returns




* ` void `
<!-- HHAPIDOC -->
