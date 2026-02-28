
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Remove all the elements from the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function clear(): Map<Tk, Tv>;
```




Future changes made to the current [` Map `](/apis/Classes/HH/Map/) ARE reflected in the returned
[` Map `](/apis/Classes/HH/Map/), and vice-versa.




## Returns




+ [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - Returns itself.




## Examples




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};
\var_dump($m);

$m->clear();
\var_dump($m);
```
<!-- HHAPIDOC -->
