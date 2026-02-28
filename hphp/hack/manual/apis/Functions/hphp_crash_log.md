
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Adds an entry to a log file that's written when server crashes




``` Hack
function hphp_crash_log(
  string $name,
  string $value,
): void;
```




This is
useful for diagnose why server crashed. For example, logged-on user's ID.




## Parameters




+ ` string $name ` - Name of the value.
+ ` string $value ` - Value to write to log.




## Returns




* ` void `
<!-- HHAPIDOC -->
