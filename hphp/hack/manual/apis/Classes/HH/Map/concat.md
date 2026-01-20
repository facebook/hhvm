
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) that is the concatenation of the values of the current
[` Map `](/apis/Classes/HH/Map/) and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): Vector<Tu>;
```




The provided [` Traversable `](/apis/Interfaces/HH/Traversable/) is concatenated to the end of the current [` Map `](/apis/Classes/HH/Map/)
to produce the returned [` Vector `](/apis/Classes/HH/Vector/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` Map `](/apis/Classes/HH/Map/).




## Returns




- [` Vector<Tu> `](/apis/Classes/HH/Vector/) - The integer-indexed concatenated [` Vector `](/apis/Classes/HH/Vector/).




## Examples




This example creates new [` Vector `](/apis/Classes/HH/Vector/)s by concatenating the values in a [` Map `](/apis/Classes/HH/Map/) with [` Traversable `](/apis/Interfaces/HH/Traversable/)s:




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
};

// Create a Vector by concating the values from $m with a Set
$v1 = $m->concat(Set {'#00ff00', '#0000ff'});

// Create a Vector by concating the values from $m with a Vector
$v2 = $m->concat(Vector {'#ffff00', '#663399'});

\var_dump($m->values()); // $m contains the value '#ff0000'
\var_dump($v1); // $v1 contains '#ff0000', '#00ff00', '#0000ff'
\var_dump($v2); // $v2 contains '#ff0000', '#ffff00', '#663399'
```
<!-- HHAPIDOC -->
