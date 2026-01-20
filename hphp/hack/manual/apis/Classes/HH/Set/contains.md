
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Determines if the specified value is in the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function contains(
  arraykey $val,
): bool;
```




## Parameters




+ ` arraykey $val `




## Returns




* ` bool ` - `` true `` if the specified value is present in the current [` Set `](/apis/Classes/HH/Set/);
  `` false `` otherwise.




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

// Prints "true", since $s contains "red"
\var_dump($s->contains('red'));

// Prints "false", since $s doesn't contain "blurple"
\var_dump($s->contains('blurple'));
```
<!-- HHAPIDOC -->
