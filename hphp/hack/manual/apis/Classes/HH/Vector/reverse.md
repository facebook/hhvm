
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Reverse the elements of the current [` Vector `](/apis/Classes/HH/Vector/) in place




``` Hack
public function reverse(): void;
```




## Returns




+ ` void `




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$v->reverse();

\var_dump($v);
```
<!-- HHAPIDOC -->
