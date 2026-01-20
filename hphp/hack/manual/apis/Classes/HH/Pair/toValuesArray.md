
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an ` varray ` containing the values from the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function toValuesArray<Tu>(): varray<Tu>;
```




This method is interchangeable with [` toVArray() `](/apis/Classes/HH/Pair/toVArray/).




## Returns




+ ` varray<Tu> ` - an `` varray `` containing the values from the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

$array = $p->toValuesArray();

\var_dump(\HH\is_any_array($array));
\var_dump($array);
```
<!-- HHAPIDOC -->
