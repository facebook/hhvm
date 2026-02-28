
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new [` Map `](/apis/Classes/HH/Map/) with the keys that are in the current [` Map `](/apis/Classes/HH/Map/), but not
in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
public function differenceByKey(
  KeyedTraversable<Tk, Tv> $traversable,
): Map<Tk, Tv>;
```




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) on which to compare the keys.




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - A [` Map `](/apis/Classes/HH/Map/) containing the keys (and associated values) of the
  current [` Map `](/apis/Classes/HH/Map/) that are not in the [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/).




## Examples




This example shows how ` differenceByKey ` can be used to get a new [` Map `](/apis/Classes/HH/Map/) with some keys excluded:




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
  'purple' => '#663399',
};

$m2 = $m->differenceByKey(Set {'red', 'green', 'blue'});

\var_dump($m2);
```
<!-- HHAPIDOC -->
