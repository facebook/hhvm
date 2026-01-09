
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retrieves information about the caller that invoked the current function or
method




``` Hack
function hphp_debug_caller_info(): darray<string, mixed>;
```




## Returns




+ ` array ` - - Returns an associative array. On success, the array will
  contain keys 'file', 'function', 'line' and optionally 'class' which
  indicate the filename, function, line number and class name (if in class
  context) of the callsite that invoked the current function or method.
<!-- HHAPIDOC -->
