
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The GTID of database returned for the current commit




``` Hack
public function recvGtid(): string;
```




This is particularly useful for ` INSERT `, `` DELETE ``, ``` UPDATE ``` statements.




## Returns




+ ` string ` - The gtid of the current commit as a `` string ``.
<!-- HHAPIDOC -->
