
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first key in the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function firstKey(): int;
```




The return will always be 0 since a [` Pair `](/apis/Classes/HH/Pair/) only has two keys, 0 and 1.




## Returns




+ ` int ` - 0




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};
\var_dump($p->firstKey());
```
<!-- HHAPIDOC -->
