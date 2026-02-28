
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable, integer-keyed map (` ImmMap `) based on the elements of
the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function toImmMap(): ImmMap<int, mixed>;
```




The keys are 0 and 1.




## Returns




+ [` ImmMap<int, `](/apis/Classes/HH/ImmMap/)`` mixed> `` - an ``` ImmMap ``` with the values of the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

$imm_map = $p->toImmMap();

\var_dump($imm_map is \HH\ImmMap<_, _>);
\var_dump($imm_map);
```
<!-- HHAPIDOC -->
