
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates a [` Map `](/docs/apis/Classes/HH/Map/) from the given [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), or an empty [` Map `](/docs/apis/Classes/HH/Map/) if
`` null `` is passed




``` Hack
public static function fromItems(
  ?Traversable<Pair<Tk, Tv>> $iterable,
): Map<Tk, Tv>;
```




This is the static method version of the [` Map::__construct() `](/docs/apis/Classes/HH/Map/__construct/) constructor.




## Parameters




+ ` ? `[` Traversable<Pair<Tk, `](/docs/apis/Interfaces/HH/Traversable/)`` Tv>> $iterable ``




## Returns




* [` Map<Tk, `](/docs/apis/Classes/HH/Map/)`` Tv> `` - A [` Map `](/docs/apis/Classes/HH/Map/) with the key/value pairs from the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/); or an
  empty [` Map `](/docs/apis/Classes/HH/Map/) if the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) is `` null ``.




## Examples




This example shows that a [` Map `](/docs/apis/Classes/HH/Map/) can be created from any [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) of key-value pairs:




``` basic-usage.hack
// Create a new Map from an array of key-value pairs
$m = Map::fromItems(varray[
  Pair {'red', '#ff0000'},
  Pair {'green', '#00ff00'},
  Pair {'blue', '#0000ff'},
  Pair {'yellow', '#ffff00'},
]);
\var_dump($m);

// Create a new Map from a Vector of key-value pairs
$m = Map::fromItems(Vector {
  Pair {'red', '#ff0000'},
  Pair {'green', '#00ff00'},
  Pair {'blue', '#0000ff'},
  Pair {'yellow', '#ffff00'},
});
\var_dump($m);

// An empty Map is created if null is provided
$m = Map::fromItems(null);
\var_dump($m);
```
<!-- HHAPIDOC -->
