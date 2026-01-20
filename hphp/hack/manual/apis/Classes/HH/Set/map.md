
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Set `](/apis/Classes/HH/Set/) containing the values after an operation has been applied
to each value in the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function map<Tu as arraykey>(
  (function(Tv): Tu) $callback,
): Set<Tu>;
```




Every value in the current [` Set `](/apis/Classes/HH/Set/) is affected by a call to [` map() `](/apis/Classes/HH/Set/map/), unlike
[` filter() `](/apis/Classes/HH/Set/filter/) where only values that meet a certain criteria are affected.




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $callback `




## Returns




- [` Set<Tu> `](/apis/Classes/HH/Set/) - a [` Set `](/apis/Classes/HH/Set/) containing the values after a user-specified operation
  is applied.




## Examples




In this example the [` Set `](/apis/Classes/HH/Set/)'s elements are mapped to the same type (`` string ``s):




``` map-to-strings.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

$capitalized = $s->map(fun('strtoupper'));
\var_dump($capitalized);

$shortened = $s->map($color ==> \substr($color, 0, 3));
\var_dump($shortened);
```




In this example the [` Set `](/apis/Classes/HH/Set/)'s elements are mapped to a different type (`` int ``s):




``` map-to-ints.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

$lengths = $s->map(fun('strlen'));
\var_dump($lengths);
```
<!-- HHAPIDOC -->
