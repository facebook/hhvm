
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns ` false `; a [` Pair `](/apis/Classes/HH/Pair/) cannot be empty




``` Hack
public function isEmpty(): bool;
```




## Returns




+ ` bool ` - `` false ``




## Examples




This example shows that a [` Pair `](/apis/Classes/HH/Pair/) can never be empty:




``` basic-usage.hack
$p = Pair {'foo', -1.5};
\var_dump($p->isEmpty());

$p = Pair {null, -1.5};
\var_dump($p->isEmpty());
```
<!-- HHAPIDOC -->
