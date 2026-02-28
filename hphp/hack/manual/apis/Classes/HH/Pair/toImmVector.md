
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable vector (` ImmVector `) containing the elements of the
current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function toImmVector(): ImmVector<mixed>;
```




## Returns




+ [` ImmVector<mixed> `](/apis/Classes/HH/ImmVector/) - an `` ImmVector `` with the elements of the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

$immutable_v = $p->toImmVector();

\var_dump($immutable_v);
```
<!-- HHAPIDOC -->
