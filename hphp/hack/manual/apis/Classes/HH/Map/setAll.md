
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), stores a value into the
current [` Map `](/apis/Classes/HH/Map/) associated with each key, overwriting the previous value
associated with the key




``` Hack
public function setAll(
  ?KeyedTraversable<Tk, Tv> $iterable,
): Map<Tk, Tv>;
```




This method is equivalent to [` Map::addAll() `](/apis/Classes/HH/Map/addAll/). If a key to set does not
exist in the Map that does exist in the [` Traversable `](/apis/Interfaces/HH/Traversable/), it is created. This
is inconsistent with, for example, the method [` Vector::setAll() `](/apis/Classes/HH/Vector/setAll/) where if
a key is not found, an exception is thrown.




Future changes made to the current [` Map `](/apis/Classes/HH/Map/) ARE reflected in the returned
[` Map `](/apis/Classes/HH/Map/), and vice-versa.




## Parameters




+ ` ? `[` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $iterable ``




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - Returns itself.




## Examples




This example shows how [` setAll() `](/apis/Classes/HH/Map/setAll/) can be used with any [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/):




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Set the values at keys 'red' and 'green'
$m->setAll(Map {
  'red' => 'rgb(255, 0, 0)',
  'green' => 'rgb(0, 255, 0)',
});

// Set the values at keys 'blue' and 'yellow' with an associative array
$m->setAll(darray[
  'blue' => 'rgb(0, 0, 255)',
  'yellow' => 'rgb(255, 255, 0)',
]);

\var_dump($m);
```
<!-- HHAPIDOC -->
