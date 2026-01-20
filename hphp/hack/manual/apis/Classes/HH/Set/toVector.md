
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a ` Vector ` of the current [` Set `](/apis/Classes/HH/Set/) values




``` Hack
public function toVector(): Vector<Tv>;
```




## Returns




+ [` Vector<Tv> `](/apis/Classes/HH/Vector/) - a `` Vector `` (integer-indexed) that contains the values of the
  current [` Set `](/apis/Classes/HH/Set/).




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

$v = $s->toVector();

\var_dump($v);
```
<!-- HHAPIDOC -->
