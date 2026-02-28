
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) that is the concatenation of the values of the current
[` Vector `](/apis/Classes/HH/Vector/) and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): Vector<Tu>;
```




The returned [` Vector `](/apis/Classes/HH/Vector/) is created from the values of the current [` Vector `](/apis/Classes/HH/Vector/),
followed by the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/).




The returned [` Vector `](/apis/Classes/HH/Vector/) is a new object; the current [` Vector `](/apis/Classes/HH/Vector/) is unchanged.
Future changes to the current [` Vector `](/apis/Classes/HH/Vector/) will not affect the returned
[` Vector `](/apis/Classes/HH/Vector/), and future changes to the returned [` Vector `](/apis/Classes/HH/Vector/) will not affect the
current [` Vector `](/apis/Classes/HH/Vector/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to concatenate with the current
  [` Vector `](/apis/Classes/HH/Vector/).




## Returns




- [` Vector<Tu> `](/apis/Classes/HH/Vector/) - A new [` Vector `](/apis/Classes/HH/Vector/) containing the values from `` $traversable ``
  concatenated to the values from the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




This example creates new [` Vector `](/apis/Classes/HH/Vector/)s by concatenating other [` Traversable `](/apis/Interfaces/HH/Traversable/)s. Unlike [` Vector::addAll() `](/apis/Classes/HH/Vector/addAll/) this method returns a new [` Vector `](/apis/Classes/HH/Vector/) (not a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>)).




``` basic-usage.hack
$v = Vector {'red'};

// Add all the values in a Set
$v1 = $v->concat(Set {'green', 'blue'});

// Add all the values in an array
$v2 = $v1->concat(varray['yellow', 'purple']);

\var_dump($v); // $v contains 'red'
\var_dump($v1); // $v1 contains 'red', 'green', 'blue'
\var_dump($v2); // $v2 contains 'red', 'green', 'blue', 'yellow', 'purple'
```
<!-- HHAPIDOC -->
