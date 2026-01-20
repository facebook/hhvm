
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns 2; a [` Pair `](/apis/Classes/HH/Pair/) always has two values




``` Hack
public function count(): int;
```




## Returns




+ ` int ` - 2




## Examples




This shows that a [` Pair `](/apis/Classes/HH/Pair/) always has a count of `` 2 ``:




``` basic-usage.hack
$p = Pair {'foo', -1.5};
\var_dump($p->count());

$p = Pair {null, null};
\var_dump($p->count());
```
<!-- HHAPIDOC -->
