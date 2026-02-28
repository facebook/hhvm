
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Map `](/apis/Classes/HH/Map/) where each value is a [` Pair `](/apis/Classes/HH/Pair/) that combines the value
of the current [` Map `](/apis/Classes/HH/Map/) and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): Map<Tk, Pair<Tv, Tu>>;
```




If the number of values of the current [` Map `](/apis/Classes/HH/Map/) are not equal to the number
of elements in the [` Traversable `](/apis/Interfaces/HH/Traversable/), then only the combined elements up to
and including the final element of the one with the least number of
elements is included.




The keys associated with the current [` Map `](/apis/Classes/HH/Map/) remain unchanged in the
returned [` Map `](/apis/Classes/HH/Map/).




## Parameters




+ [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to use to combine with the
  elements of the current [` Map `](/apis/Classes/HH/Map/).




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Pair<Tv, Tu>> `` - The [` Map `](/apis/Classes/HH/Map/) that combines the values of the current [` Map `](/apis/Classes/HH/Map/) with
  the provided [` Traversable `](/apis/Interfaces/HH/Traversable/).




## Examples




This example shows how ` zip ` combines the values of the [` Map `](/apis/Classes/HH/Map/) and another [` Traversable `](/apis/Interfaces/HH/Traversable/). The resulting [` Map `](/apis/Classes/HH/Map/) `` $labeled_colors `` has three elements because ``` $labels ``` doesn't have a fourth element to pair with ```` $m ````.




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
  'purple' => '#663399',
};

$labels = Vector {'My Favorite', 'My 2nd Favorite', 'My 3rd Favorite'};
$labeled_colors = $m->zip($labels);

\var_dump($labeled_colors->count()); // 3

foreach ($labeled_colors as $color => $pair) {
  $hex_code = $pair[0];
  $label = $pair[1];
  echo "{$label}: {$color} ($hex_code)\n";
}
```
<!-- HHAPIDOC -->
