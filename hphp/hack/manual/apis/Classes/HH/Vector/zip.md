
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) where each element is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
element of the current [` Vector `](/apis/Classes/HH/Vector/) and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): Vector<Pair<Tv, Tu>>;
```




If the number of elements of the [` Vector `](/apis/Classes/HH/Vector/) are not equal to the number of
elements in the [` Traversable `](/apis/Interfaces/HH/Traversable/), then only the combined elements up to and
including the final element of the one with the least number of elements
is included.




## Parameters




+ [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to use to combine with the
  elements of the current [` Vector `](/apis/Classes/HH/Vector/).




## Returns




* [` Vector<Pair<Tv, `](/apis/Classes/HH/Vector/)`` Tu>> `` - A [` Vector `](/apis/Classes/HH/Vector/) that combines the values of the current [` Vector `](/apis/Classes/HH/Vector/)
  with the provided [` Traversable `](/apis/Interfaces/HH/Traversable/).




## Examples




This example shows how ` zip ` combines the values of the [` Vector `](/apis/Classes/HH/Vector/) and another [` Traversable `](/apis/Interfaces/HH/Traversable/). The resulting [` Vector `](/apis/Classes/HH/Vector/) `` $labeled_colors `` has three elements because ``` $labels ``` doesn't have a fourth element to pair with ```` $v ````.




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$labels = Vector {'My Favorite', 'My 2nd Favorite', 'My 3rd Favorite'};
$labeled_colors = $v->zip($labels);

\var_dump($labeled_colors->count()); // 3

foreach ($labeled_colors as list($color, $label)) {
  echo $label.': '.$color."\n";
}
```
<!-- HHAPIDOC -->
