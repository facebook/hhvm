
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates a [` Vector `](/apis/Classes/HH/Vector/) from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty [` Vector `](/apis/Classes/HH/Vector/) if
`` null `` is passed




``` Hack
public static function fromItems(
  ?Traversable<Tv> $iterable,
): Vector<Tv>;
```




This is the static method version of the [` Vector::__construct() `](/apis/Classes/HH/Vector/__construct/)
constructor.




## Parameters




+ ` ? `[` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $iterable ``




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) with the values from the [` Traversable `](/apis/Interfaces/HH/Traversable/); or an empty
  [` Vector `](/apis/Classes/HH/Vector/) if the [` Traversable `](/apis/Interfaces/HH/Traversable/) is `` null ``.




## Examples




``` basic-usage.hack
// Create a new Vector from an array
$v = Vector::fromItems(varray['red', 'green', 'blue', 'yellow']);
\var_dump($v);

// Create a new Vector from a Set
$v = Vector::fromItems(Set {'red', 'green', 'blue', 'yellow'});
\var_dump($v);

// Create a new Vector from the values of a Map
$v = Vector::fromItems(Map {
  'red' => 'red',
  'green' => 'green',
  'blue' => 'blue',
  'yellow' => 'yellow',
});
\var_dump($v);

// An empty Vector is created if null is provided
$v = Vector::fromItems(null);
\var_dump($v);
```
<!-- HHAPIDOC -->
