
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Flush all the currently buffered output, so that the main thread can read
it with pagelet_server_task_result()




``` Hack
function pagelet_server_flush(): void;
```




This is only meaningful in a pagelet
thread.




## Returns




+ ` void `
<!-- HHAPIDOC -->
