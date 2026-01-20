
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an integer-keyed ` Map ` based on the values of the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function toMap(): Map<int, Tv>;
```




## Returns




+ [` Map<int, `](/apis/Classes/HH/Map/)`` Tv> `` - A ``` Map ``` that has the integer keys and associated values of the
  current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$map = $v->toMap();

\var_dump($map is \HH\Map<_, _>);
\var_dump($map->keys());
\var_dump($map);
```
<!-- HHAPIDOC -->
