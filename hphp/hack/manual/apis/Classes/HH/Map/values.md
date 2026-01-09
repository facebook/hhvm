
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/docs/apis/Classes/HH/Vector/) containing the values of the current [` Map `](/docs/apis/Classes/HH/Map/)




``` Hack
public function values(): Vector<Tv>;
```




This method is interchangeable with [` toVector() `](/docs/apis/Classes/HH/Map/toVector/).




## Returns




+ [` Vector<Tv> `](/docs/apis/Classes/HH/Vector/) - a [` Vector `](/docs/apis/Classes/HH/Vector/) containing the values of the current [` Map `](/docs/apis/Classes/HH/Map/).




## Examples




This example shows how [` values() `](/docs/apis/Classes/HH/Map/values/) is identical to [` toVector() `](/docs/apis/Classes/HH/Map/toVector/). It returns a new [` Vector `](/docs/apis/Classes/HH/Vector/) of `` $m ``'s values, so mutating this new [` Vector `](/docs/apis/Classes/HH/Vector/) doesn't affect the original [` Map `](/docs/apis/Classes/HH/Map/).




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Get a Vector of $m's values
$v = $m->values();

// Modify $v by adding an element
$v->add('#663399');
\var_dump($v);

// The original Map $m doesn't include the value '#663399'
\var_dump($m);
```
<!-- HHAPIDOC -->
