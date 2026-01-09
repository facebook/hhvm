
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For every element in the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), add a key/value pair into
the current [` Map `](/docs/apis/Classes/HH/Map/)




``` Hack
public function addAll(
  ?Traversable<Pair<Tk, Tv>> $iterable,
): Map<Tk, Tv>;
```




This method is equivalent to [` Map::setAll() `](/docs/apis/Classes/HH/Map/setAll/). If a key in the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)
exists in the [` Map `](/docs/apis/Classes/HH/Map/), then the value associated with that key in the [` Map `](/docs/apis/Classes/HH/Map/)
is overwritten.




Future changes made to the current [` Map `](/docs/apis/Classes/HH/Map/) ARE reflected in the returned
[` Map `](/docs/apis/Classes/HH/Map/), and vice-versa.




## Parameters




+ ` ? `[` Traversable<Pair<Tk, `](/docs/apis/Interfaces/HH/Traversable/)`` Tv>> $iterable ``




## Returns




* [` Map<Tk, `](/docs/apis/Classes/HH/Map/)`` Tv> `` - Returns itself.




## Examples




The following example adds a collection of key-value pairs to the [` Map `](/docs/apis/Classes/HH/Map/) `` $m `` and also adds multiple collections of key-value pairs to ``` $m ``` through chaining. Since [` Map::addAll() `](/docs/apis/Classes/HH/Map/addAll/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $m ` itself, you can chain a bunch of [` addAll() `](/docs/apis/Classes/HH/Map/addAll/) calls together.




``` basic-usage.hack
$m = Map {};

// Add all the key-value pairs in an array
$m->addAll(varray[Pair {'red', '#ff0000'}]);

// Map::addAll returns the Map so it can be chained
$m->addAll(Vector {
  Pair {'green', '#00ff00'},
  Pair {'blue', '#0000ff'},
})
  ->addAll(ImmVector {
    Pair {'yellow', '#ffff00'},
    Pair {'purple', '#663399'},
  });

\var_dump($m);
```
<!-- HHAPIDOC -->
