
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Throws an exception unless the current [` Set `](/apis/Classes/HH/Set/) or the [` Traversable `](/apis/Interfaces/HH/Traversable/) is
empty




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): Set<HH\nothing>;
```




Since [` Set `](/apis/Classes/HH/Set/)s only support integers or strings as values, we cannot have
a [` Pair `](/apis/Classes/HH/Pair/) as a [` Set `](/apis/Classes/HH/Set/) value. So in order to avoid an
`` InvalidArgumentException ``, either the current [` Set `](/apis/Classes/HH/Set/) or the [` Traversable `](/apis/Interfaces/HH/Traversable/)
must be empty so that we actually return an empty [` Set `](/apis/Classes/HH/Set/).




## Parameters




+ [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to use to combine with the
  elements of the current [` Set `](/apis/Classes/HH/Set/).




## Returns




* [` Set<HH\nothing> `](/apis/Classes/HH/Set/) - The [` Set `](/apis/Classes/HH/Set/) that combines the values of the current [` Set `](/apis/Classes/HH/Set/) with
  the provided [` Traversable `](/apis/Interfaces/HH/Traversable/); one of these must be empty or an
  exception is thrown.




## Examples




This example shows that ` zip ` won't thrown an `` Exception `` if at least one of the current [` Set `](/apis/Classes/HH/Set/) or the `` $traversable `` is empty:




``` empty-usage.hack
// The $traversable is empty so the result will be empty
$s = Set {'red', 'green', 'blue', 'yellow'};
$zipped = $s->zip(Vector {});
\var_dump($zipped);

// The Set $s is empty so the result will be empty
$s = Set {};
$zipped = $s->zip(Vector {'My Favorite', 'My Second Favorite'});
\var_dump($zipped);
```




This example shows that ` zip ` will throw an `` Exception `` if the result is non-empty:




``` nonempty-exception.hack
$s = Set {'red', 'green', 'blue', 'yellow'};
$zipped = $s->zip(Vector {'My Favorite', 'My Second Favorite'});
\var_dump($zipped);
```
<!-- HHAPIDOC -->
