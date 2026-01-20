
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), stores a value into the
current [` Vector `](/apis/Classes/HH/Vector/) associated with each key, overwriting the previous value
associated with the key




``` Hack
public function setAll(
  ?KeyedTraversable<int, Tv> $iterable,
): Vector<Tv>;
```




If a key is not present the current [` Vector `](/apis/Classes/HH/Vector/) that is present in the
[` Traversable `](/apis/Interfaces/HH/Traversable/), an exception is thrown. If you want to add a value even if a
key is not present, use [` addAll() `](/apis/Classes/HH/Vector/addAll/).




Future changes made to the current [` Vector `](/apis/Classes/HH/Vector/) ARE reflected in the
returned [` Vector `](/apis/Classes/HH/Vector/), and vice-versa.




## Parameters




+ ` ? `[` KeyedTraversable<int, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $iterable ``




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - Returns itself.




## Examples




This example shows how [` setAll() `](/apis/Classes/HH/Vector/setAll/) can be used with any [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/):




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Set the elements at 0 and 1
$v->setAll(Vector {'foo', 'bar'});
\var_dump($v);

// Set the elements at 2 and 3
$v->setAll(Map {
  2 => 'baz',
  3 => 'qux',
});
\var_dump($v);
```
<!-- HHAPIDOC -->
