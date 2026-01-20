
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Provides the number of elements in the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function count(): int;
```




## Returns




+ ` int ` - The number of elements in the current [` Map `](/apis/Classes/HH/Map/).




## Examples




``` basic-usage.hack
$m = Map {};
\var_dump($m->count());

$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};
\var_dump($m->count());
```
<!-- HHAPIDOC -->
