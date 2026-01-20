
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), add the value into the
current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function addAll(
  ?Traversable<Tv> $iterable,
): Set<Tv>;
```




Future changes made to the original [` Set `](/apis/Classes/HH/Set/) ARE reflected in the returned
[` Set `](/apis/Classes/HH/Set/), and vice-versa.




## Parameters




+ ` ? `[` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $iterable ``




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - Returns itself.




## Examples




The following example adds a collection of values to the [` Set `](/apis/Classes/HH/Set/) `` $s `` and also adds multiple collections of values to ``` $s ``` through chaining. Since [` Set::addAll() `](/apis/Classes/HH/Set/addAll/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $s ` itself, you can chain a bunch of [` addAll() `](/apis/Classes/HH/Set/addAll/) calls together, and that will add all those collection of values to `` $s ``.




``` basic-usage.hack
$s = Set {};

// Add all the values in a Vector
$s->addAll(Vector {'a', 'b'});

// Add all the values in a Set
$s->addAll(Set {'c', 'd'});

// Add all the values in a Map
$s->addAll(Map {'foo' => 'e', 'bar' => 'f'});

// Add all the values in an array
$s->addAll(varray['g', 'h']);

// Set::addAll returns the Set so it can be chained
$s->addAll(ImmSet {'i', 'j'})
  ->addAll(ImmVector {'k', 'l'});

\var_dump($s);
```
<!-- HHAPIDOC -->
