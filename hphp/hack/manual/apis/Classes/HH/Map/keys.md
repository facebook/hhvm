
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the keys of the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function keys(): Vector<Tk>;
```




## Returns




+ [` Vector<Tk> `](/apis/Classes/HH/Vector/) - a [` Vector `](/apis/Classes/HH/Vector/) containing the keys of the current [` Map `](/apis/Classes/HH/Map/).




## Examples




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};
\var_dump($m->keys());
```
<!-- HHAPIDOC -->
