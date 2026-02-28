
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), append a value into this
[` Vector `](/apis/Classes/HH/Vector/), assigning the next available integer key for each




``` Hack
public function addAll(
  ?Traversable<Tv> $iterable,
): Vector<Tv>;
```




If you want to overwrite the values for existing keys, use [` setAll() `](/apis/Classes/HH/Vector/setAll/).




Future changes made to the current [` Vector `](/apis/Classes/HH/Vector/) ARE reflected in the
returned [` Vector `](/apis/Classes/HH/Vector/), and vice-versa.




## Parameters




+ ` ? `[` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $iterable ``




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - Returns itself.




## Examples




The following example adds a collection of values to the [` Vector `](/apis/Classes/HH/Vector/) `` $v `` and also adds multiple collections of values to ``` $v ``` through chaining. Since [` Vector::addAll() `](/apis/Classes/HH/Vector/addAll/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $v ` itself, you can chain a bunch of [` addAll() `](/apis/Classes/HH/Vector/addAll/) calls together, and that will add all those collection of values to `` $v ``.




``` basic-usage.hack
$v = Vector {};

// Add all the values in a Set
$v->addAll(Set {'a', 'b'});

// Add all the values in a Vector
$v->addAll(Vector {'c', 'd'});

// Add all the values in a Map
$v->addAll(Map {'foo' => 'e', 'bar' => 'f'});

// Add all the values in an array
$v->addAll(varray['g', 'h']);

// Vector::addAll returns the Vector so it can be chained
$v->addAll(ImmSet {'i', 'j'})
  ->addAll(ImmVector {'k', 'l'});

\var_dump($v);
```
<!-- HHAPIDOC -->
