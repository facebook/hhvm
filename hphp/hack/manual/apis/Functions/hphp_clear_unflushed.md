
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Clears any output contents that have not been flushed to networked




``` Hack
function hphp_clear_unflushed(): void;
```




This is useful when handling a fatal error. Before displaying a customized
PHP page, one may call this function to clear previously written content, so
to replay what will be displayed.




## Returns




+ ` void `
<!-- HHAPIDOC -->
