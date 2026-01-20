
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable map (` ImmMap `) based on the values of the current
[` Set `](/apis/Classes/HH/Set/)




``` Hack
public function toImmMap(): ImmMap<arraykey, Tv>;
```




Each key of the ` Map ` will be the same as its value.




## Returns




+ [` ImmMap<arraykey, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - an ``` ImmMap ``` that that contains the values of the current [` Set `](/apis/Classes/HH/Set/),
  with each key of the Map being the same as its value.




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

$imm_map = $s->toImmMap();

\var_dump($imm_map is \HH\ImmMap<_, _>);
\var_dump($imm_map);
```
<!-- HHAPIDOC -->
