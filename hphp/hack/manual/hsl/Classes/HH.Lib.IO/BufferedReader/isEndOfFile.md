
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

If we are known to have reached the end of the file




``` Hack
public function isEndOfFile(): bool;
```




This function is best-effort: ` true ` is reliable, but `` false `` is more of
'maybe'. For example, if called on an open socket with no data available,
it will return ``` false ```; it is then possible that a future read will:

+ return data if the other send sends some more
+ block forever, or until timeout if set
+ return the empty string if the socket closes the connection




Additionally, helpers such as ` readUntil ` may fail with `` EPIPE ``.




## Returns




* ` bool `
<!-- HHAPIDOC -->
