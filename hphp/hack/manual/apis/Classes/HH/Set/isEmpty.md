
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Checks if the current [` Set `](/apis/Classes/HH/Set/) is empty




``` Hack
public function isEmpty(): bool;
```




## Returns




+ ` bool ` - `` true `` if the current [` Set `](/apis/Classes/HH/Set/) is empty; `` false `` otherwise.




## Examples




``` basic-usage.hack
$s = Set {};
\var_dump($s->isEmpty());

$s = Set {'red', 'green', 'blue', 'yellow'};
\var_dump($s->isEmpty());
```
<!-- HHAPIDOC -->
