
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an integer-keyed ` Map ` based on the elements of the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function toMap(): Map<int, mixed>;
```




The keys are 0 and 1.




## Returns




+ [` Map<int, `](/apis/Classes/HH/Map/)`` mixed> `` - an integer-keyed ``` Map ``` with the values of the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

$map = $p->toMap();

\var_dump($map is \HH\Map<_, _>);
\var_dump($map);
```
<!-- HHAPIDOC -->
