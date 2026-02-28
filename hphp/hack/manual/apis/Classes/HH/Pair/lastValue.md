
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last value in the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function lastValue(): Tv2;
```




## Returns




+ ` Tv2 ` - The last value in the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};
\var_dump($p->lastValue());
```
<!-- HHAPIDOC -->
