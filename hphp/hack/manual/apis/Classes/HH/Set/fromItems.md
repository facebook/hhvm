
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates a [` Set `](/docs/apis/Classes/HH/Set/) from the given [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), or an empty [` Set `](/docs/apis/Classes/HH/Set/) if `` null ``
is passed




``` Hack
public static function fromItems(
  ?Traversable<Tv> $iterable,
): Set<Tv>;
```




This is the static method version of the [` Set::__construct() `](/docs/apis/Classes/HH/Set/__construct/) constructor.




## Parameters




+ ` ? `[` Traversable<Tv> `](/docs/apis/Interfaces/HH/Traversable/)`` $iterable ``




## Returns




* [` Set<Tv> `](/docs/apis/Classes/HH/Set/) - A [` Set `](/docs/apis/Classes/HH/Set/) with the values from the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/); or an empty [` Set `](/docs/apis/Classes/HH/Set/)
  if the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) is `` null ``.




## Examples




``` basic-usage.hack
// Create a new Set from an array
$s = Set::fromItems(varray['red', 'green', 'red', 'blue', 'blue', 'yellow']);
\var_dump($s);

// Create a new Set from a Vector
$s = Set::fromItems(Vector {'red', 'green', 'red', 'blue', 'blue', 'yellow'});
\var_dump($s);

// Create a new Set from the values of a Map
$s = Set::fromItems(Map {
  'red1' => 'red',
  'green' => 'green',
  'red2' => 'red',
  'blue1' => 'blue',
  'blue2' => 'blue',
  'yellow' => 'yellow',
});
\var_dump($s);
```
<!-- HHAPIDOC -->
