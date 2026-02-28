
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes the values in the current [` Set `](/apis/Classes/HH/Set/) that are also in the [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function removeAll(
  Traversable<Tv> $iterable,
): Set<Tv>;
```




If a value in the [` Traversable `](/apis/Interfaces/HH/Traversable/) doesn't exist in the current [` Set `](/apis/Classes/HH/Set/), that
value in the [` Traversable `](/apis/Interfaces/HH/Traversable/) is ignored.




Future changes made to the current [` Set `](/apis/Classes/HH/Set/) ARE reflected in the returned
[` Set `](/apis/Classes/HH/Set/), and vice-versa.




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $iterable ``




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - Returns itself.




## Examples




This example removes multiple values from a [` Set `](/apis/Classes/HH/Set/) and shows that the list of values to be removed can contain duplicates:




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

$s->removeAll(Vector {
  'red',
  'blue',
  'red',
});

\var_dump($s);
```
<!-- HHAPIDOC -->
