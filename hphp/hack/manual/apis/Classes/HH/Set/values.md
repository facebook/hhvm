
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function values(): Vector<Tv>;
```




This method is interchangeable with [` toVector() `](/apis/Classes/HH/Set/toVector/) and [` keys() `](/apis/Classes/HH/Set/keys/).




## Returns




+ [` Vector<Tv> `](/apis/Classes/HH/Vector/) - a [` Vector `](/apis/Classes/HH/Vector/) (integer-indexed) containing the values of the
  current [` Set `](/apis/Classes/HH/Set/).




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

$v = $s->values();

\var_dump($v);
```
<!-- HHAPIDOC -->
