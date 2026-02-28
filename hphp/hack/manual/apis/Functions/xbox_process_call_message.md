
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This function is invoked by the xbox facility to start an xbox call task




``` Hack
function xbox_process_call_message(
  string $msg,
): mixed;
```




This function is not intended to be called directly by user code.




## Parameters




+ ` string $msg ` - The call message.




## Returns




* ` mixed ` - - The return value of the xbox call task.
<!-- HHAPIDOC -->
