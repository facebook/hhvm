
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a ` Vector ` with the values of the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function toVector(): Vector<Tv>;
```




## Returns




+ [` Vector<Tv> `](/apis/Classes/HH/Vector/) - a `` Vector `` that contains the values of the current [` Map `](/apis/Classes/HH/Map/).




## Examples




This example shows how [` toVector() `](/apis/Classes/HH/Map/toVector/) returns a `` Vector `` of ``` $m ```'s values, so mutating this new ```` Vector ```` doesn't affect the original [` Map `](/apis/Classes/HH/Map/).




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Make a deep Vector copy of the values of $m
$v = $m->toVector();

// Modify $v by adding an element
$v->add('#663399');
\var_dump($v);

// The original Map $m doesn't include the value '#663399'
\var_dump($m);
```
<!-- HHAPIDOC -->
