
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a deep copy of the current ` Map `




``` Hack
public function toMap(): Map<Tk, Tv>;
```




## Returns




+ [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - a ``` Map ``` that is a deep copy of the current ```` Map ````.




## Examples




This example shows that ` toMap ` returns a deep copy of the `` Map `` ``` $m ```. Mutating the new ```` Map ```` ````` $m2 ````` doesn't affect the original `````` Map ``````.




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

$m2 = $m->toMap();
$m2->add(Pair {'purple', '#663399'});

\var_dump($m);
\var_dump($m2);
```
<!-- HHAPIDOC -->
