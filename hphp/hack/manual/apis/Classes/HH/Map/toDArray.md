
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a darray built from the keys and values from this Map




``` Hack
public function toDArray(): darray<Tk, Tv>;
```




## Returns




+ ` darray `




## Examples




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
  'purple' => '#663399',
};

$array = $m->toDArray();

\var_dump(\HH\is_any_array($array));
\var_dump($array);
```
<!-- HHAPIDOC -->
