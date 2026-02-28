
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns timestamps of different request events




``` Hack
function hphp_get_timers(
  bool $get_as_float = true,
): mixed;
```




## Parameters




+ ` bool $get_as_float = true ` - same as in microtime() to specify output
  format, except it defaults to true for float format.




## Returns




* ` mixed ` - - An array of three timestamps: 'queue', the time a request
  is received and queued up; 'process-wall', the wall clock time a request
  starts to get processed; and 'process-cpu', the CPU clock time a request
  starts to get processed.
<!-- HHAPIDOC -->
