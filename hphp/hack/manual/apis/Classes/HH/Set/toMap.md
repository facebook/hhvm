
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a ` Map ` based on the values of the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function toMap(): Map<arraykey, Tv>;
```




Each key of the ` Map ` will be the same as its value.




## Returns




+ [` Map<arraykey, `](/apis/Classes/HH/Map/)`` Tv> `` - a ``` Map ``` that that contains the values of the current [` Set `](/apis/Classes/HH/Set/), with
  each key of the `` Map `` being the same as its value.




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

$map = $s->toMap();

\var_dump($map is \HH\Map<_, _>);
\var_dump($map);
```
<!-- HHAPIDOC -->
