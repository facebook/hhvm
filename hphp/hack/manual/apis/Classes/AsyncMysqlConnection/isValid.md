
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Checks if the data inside [` AsyncMysqlConnection `](/apis/Classes/AsyncMysqlConnection/) object is valid




``` Hack
public function isValid(): bool;
```




For
example, during a timeout in a query, the MySQL connection gets closed.




## Returns




+ ` bool ` - `` true `` if MySQL resource is valid and can be accessed;
  ``` false ``` otherwise.
<!-- HHAPIDOC -->
