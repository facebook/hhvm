
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the keys of the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function keys(): Vector<int>;
```




## Returns




+ [` Vector<int> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) containing the integer keys of the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};
\var_dump($v->keys());
```
<!-- HHAPIDOC -->
