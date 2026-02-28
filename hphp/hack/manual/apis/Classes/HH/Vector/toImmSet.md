
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable set (` ImmSet `) based on the values of the current
[` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function toImmSet(): ImmSet<Tv>;
```




## Returns




+ [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - An `` ImmSet `` containing the unique values of the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




This example shows that converting a [` Vector `](/apis/Classes/HH/Vector/) to an `` ImmSet `` also removes duplicate values:




``` basic-usage.hack
// This Vector contains repetitions of 'red' and 'blue'
$v = Vector {'red', 'green', 'red', 'blue', 'red', 'yellow', 'blue'};

$imm_set = $v->toImmSet();

\var_dump($imm_set is \HH\ImmSet<_>);
\var_dump($imm_set);
```
<!-- HHAPIDOC -->
