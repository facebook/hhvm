
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a ` Vector ` containing the elements of the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function toVector(): Vector<mixed>;
```




The returned ` Vector ` will, of course, be mutable.




## Returns




+ [` Vector<mixed> `](/apis/Classes/HH/Vector/) - a `` Vector `` with the elements of the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

$v = $p->toVector();
\var_dump($v);
```
<!-- HHAPIDOC -->
