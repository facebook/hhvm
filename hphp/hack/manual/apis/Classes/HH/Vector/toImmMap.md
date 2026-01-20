
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable, integer-keyed map (` ImmMap `) based on the values of
the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function toImmMap(): ImmMap<int, Tv>;
```




## Returns




+ [` ImmMap<int, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - An ``` ImmMap ``` that has the integer keys and associated values
  of the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$imm_map = $v->toImmMap();

\var_dump($imm_map is \HH\ImmMap<_, _>);
\var_dump($imm_map->keys());
\var_dump($imm_map);
```
<!-- HHAPIDOC -->
