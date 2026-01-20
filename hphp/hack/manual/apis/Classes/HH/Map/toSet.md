
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a ` Set ` based on the values of the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function toSet(): Set<Tv>;
```




## Returns




+ [` Set<Tv> `](/apis/Classes/HH/Set/) - a `` Set `` with the current values of the current [` Map `](/apis/Classes/HH/Map/).




## Examples




This example shows that converting a [` Map `](/apis/Classes/HH/Map/) to a `` Set `` also removes duplicate values:




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

$set = $m->toSet();

\var_dump($set is \HH\Set<_>);
\var_dump($set);
```
<!-- HHAPIDOC -->
