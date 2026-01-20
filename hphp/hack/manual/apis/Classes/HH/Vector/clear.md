
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes all the elements from the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function clear(): Vector<Tv>;
```




Future changes made to the current [` Vector `](/apis/Classes/HH/Vector/) ARE reflected in the
returned [` Vector `](/apis/Classes/HH/Vector/), and vice-versa.




## Returns




+ [` Vector<Tv> `](/apis/Classes/HH/Vector/) - Returns itself.




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};
\var_dump($v);

$v->clear();
\var_dump($v);
```
<!-- HHAPIDOC -->
