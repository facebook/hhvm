
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an ` array ` containing the values from the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function toValuesArray(): varray<Tv>;
```




## Returns




+ ` varray<Tv> ` - an integer-indexed `` array `` containing the values from the
  current [` Map `](/apis/Classes/HH/Map/).




## Examples




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

$array = $m->toValuesArray();

\var_dump(\HH\is_any_array($array));
\var_dump($array);
```
<!-- HHAPIDOC -->
