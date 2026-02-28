
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the number of elements in the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function count(): int;
```




## Returns




+ ` int ` - The number of elements in the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




``` basic-usage.hack
$v = Vector {};
\var_dump($v->count());

$v = Vector {'red', 'green', 'blue', 'yellow'};
\var_dump($v->count());
```
<!-- HHAPIDOC -->
