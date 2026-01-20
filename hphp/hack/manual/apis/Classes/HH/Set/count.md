
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Provides the number of elements in the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function count(): int;
```




## Returns




+ ` int ` - The number of elements in the current [` Set `](/apis/Classes/HH/Set/).




## Examples




``` basic-usage.hack
$s = Set {};
\var_dump($s->count());

$s = Set {'red', 'green', 'blue', 'yellow'};
\var_dump($s->count());
```
<!-- HHAPIDOC -->
