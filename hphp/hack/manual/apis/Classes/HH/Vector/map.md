
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the results of applying an operation to each
value in the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $callback,
): Vector<Tu>;
```




[` map() `](/apis/Classes/HH/Vector/map/)'s result contains a value for every value in the current [` Vector `](/apis/Classes/HH/Vector/);
unlike [` filter() `](/apis/Classes/HH/Vector/filter/), where only values that meet a certain criterion are
included in the resulting [` Vector `](/apis/Classes/HH/Vector/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $callback `




## Returns




- [` Vector<Tu> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) containing the results of applying a user-specified
  operation to each value of the current [` Vector `](/apis/Classes/HH/Vector/) in turn.




## Examples




In this example the [` Vector `](/apis/Classes/HH/Vector/)'s elements are mapped to the same type (`` string ``s):




``` map-to-strings.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$capitalized = $v->map(fun('strtoupper'));
\var_dump($capitalized);

$shortened = $v->map($color ==> \substr($color, 0, 3));
\var_dump($shortened);
```




In this example the [` Vector `](/apis/Classes/HH/Vector/)'s elements are mapped to a different type (`` int ``s):




``` map-to-ints.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$lengths = $v->map(fun('strlen'));
\var_dump($lengths);
```
<!-- HHAPIDOC -->
