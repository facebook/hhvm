
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a deep copy of the current ` Set `




``` Hack
public function toSet(): Set<Tv>;
```




## Returns




+ [` Set<Tv> `](/apis/Classes/HH/Set/) - a `` Set `` that is a deep copy of the current ``` Set ```.




## Examples




This example shows that ` toSet ` returns a deep copy of the `` Set ``:




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

// Make a deep copy of Set $s
$s2 = $s->toSet();

// Modify $s2 by adding an element
$s2->add('purple');
\var_dump($s2);

// The original Set $s doesn't include 'purple'
\var_dump($s);
```
<!-- HHAPIDOC -->
