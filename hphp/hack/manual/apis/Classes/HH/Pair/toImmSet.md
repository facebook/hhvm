
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable set (` ImmSet `) with the values of the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function toImmSet(): ImmSet<arraykey, mixed>;
```




## Returns




+ [` ImmSet<arraykey, `](/apis/Classes/HH/ImmSet/)`` mixed> `` - an ``` ImmSet ``` with the current values of the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




This example shows that converting a [` Pair `](/apis/Classes/HH/Pair/) to an `` ImmSet `` also removes duplicate values:




``` basic-usage.hack
// This Pair contains 'foo' twice
$p = Pair {'foo', 'foo'};

$imm_set = $p->toImmSet();
\var_dump($imm_set);
```




This example shows that converting a [` Pair `](/apis/Classes/HH/Pair/) to an `` ImmSet `` will throw a fatal error if the [` Pair `](/apis/Classes/HH/Pair/) contains a value that's not a `` string `` or an ``` int ```:




``` runtime-fatal.hack
$p = Pair {'foo', -1.5};

/* HH_FIXME[4323] Fatal error will be thrown here */
$imm_set = $p->toImmSet();

\var_dump($imm_set);
```
<!-- HHAPIDOC -->
