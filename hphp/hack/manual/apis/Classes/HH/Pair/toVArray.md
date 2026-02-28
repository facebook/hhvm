
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
public function toVArray(): varray;
```




## Returns




+ ` varray `




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

$array = $p->toVArray();

\var_dump(\HH\is_any_array($array));
\var_dump($array);
```
<!-- HHAPIDOC -->
