
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) that is the concatenation of the values of the current
[` Set `](/apis/Classes/HH/Set/) and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): Vector<Tu>;
```




The values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/) is concatenated to the end of the
current [` Set `](/apis/Classes/HH/Set/) to produce the returned [` Vector `](/apis/Classes/HH/Vector/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` Set `](/apis/Classes/HH/Set/).




## Returns




- [` Vector<Tu> `](/apis/Classes/HH/Vector/) - The concatenated [` Vector `](/apis/Classes/HH/Vector/).




## Examples




This example creates new [` Set `](/apis/Classes/HH/Set/)s by concatenating other [` Traversable `](/apis/Interfaces/HH/Traversable/)s. Unlike [` Set::addAll() `](/apis/Classes/HH/Set/addAll/) this method returns a new [` Set `](/apis/Classes/HH/Set/) (not a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>)).




``` basic-usage.hack
$s = Set {'red'};

// Add all the values in a Vector
$s1 = $s->concat(Vector {'green', 'blue'});

// Add all the values in an array
$s2 = $s1->concat(varray['yellow', 'purple']);

\var_dump($s); // $s contains 'red'
\var_dump($s1); // $s1 contains 'red', 'green', 'blue'
\var_dump($s2); // $s2 contains 'red', 'green', 'blue', 'yellow', 'purple'
```
<!-- HHAPIDOC -->
