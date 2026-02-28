
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes the specified value from the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function remove(
  Tv $val,
): Set<Tv>;
```




Future changes made to the current [` Set `](/apis/Classes/HH/Set/) ARE reflected in the returned
[` Set `](/apis/Classes/HH/Set/), and vice-versa.




## Parameters




+ ` Tv $val `




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - Returns itself.




## Examples




This example shows that removing a value that doesn't exist in the [` Set `](/apis/Classes/HH/Set/) has no effect:




``` basic-usage.hack
$s = Set {'red', 'green'};

// Remove 'red' from the Set
$s->remove('red');
\var_dump($s);

// Remove 'red' again (has no effect)
$s->remove('red');
\var_dump($s);
```
<!-- HHAPIDOC -->
