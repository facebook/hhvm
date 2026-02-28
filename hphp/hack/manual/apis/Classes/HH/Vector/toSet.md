
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a ` Set ` based on the values of the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function toSet(): Set<Tv>;
```




## Returns




+ [` Set<Tv> `](/apis/Classes/HH/Set/) - A `` Set `` containing the unique values of the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




This example shows that converting a [` Vector `](/apis/Classes/HH/Vector/) to a `` Set `` also removes duplicate values:




``` basic-usage.hack
// This Vector contains repetitions of 'red' and 'blue'
$v = Vector {'red', 'green', 'red', 'blue', 'red', 'yellow', 'blue'};

$set = $v->toSet();

\var_dump($set is \HH\Set<_>);
\var_dump($set);
```
<!-- HHAPIDOC -->
