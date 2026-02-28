
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable set (` ImmSet `) based on the values of the current
[` Map `](/apis/Classes/HH/Map/)




``` Hack
public function toImmSet(): ImmSet<Tv>;
```




## Returns




+ [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - an `` ImmSet `` with the current values of the current [` Map `](/apis/Classes/HH/Map/).




## Examples




This example shows that converting a [` Map `](/apis/Classes/HH/Map/) to an `` ImmSet `` also removes duplicate values:




``` basic-usage.hack
// This Map contains repetitions of the hex codes for 'red' and 'blue'
$m = Map {
  'red' => '#ff0000',
  'also red' => '#ff0000',
  'green' => '#00ff00',
  'another red' => '#ff0000',
  'blue' => '#0000ff',
  'another blue' => '#0000ff',
  'yellow' => '#ffff00',
};

$imm_set = $m->toImmSet();

\var_dump($imm_set is \HH\ImmSet<_>);
\var_dump($imm_set);
```
<!-- HHAPIDOC -->
