
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Remove all the elements from the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function clear(): Set<Tv>;
```




Future changes made to the current [` Set `](/apis/Classes/HH/Set/) ARE reflected in the returned
[` Set `](/apis/Classes/HH/Set/), and vice-versa.




## Returns




+ [` Set<Tv> `](/apis/Classes/HH/Set/) - Returns itself.




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};
\var_dump($s);

$s->clear();
\var_dump($s);
```
<!-- HHAPIDOC -->
