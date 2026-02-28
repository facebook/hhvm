
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first value in the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function firstValue(): ?Tv;
```




## Returns




+ ` ?Tv ` - The first value in the current [` Map `](/apis/Classes/HH/Map/),  or `` null `` if the [` Map `](/apis/Classes/HH/Map/) is
  empty.




## Examples




The following example gets the first value from a [` Map `](/apis/Classes/HH/Map/). An empty [` Map `](/apis/Classes/HH/Map/) will return `` null `` as its first value.




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};
\var_dump($m->firstValue());

$m = Map {};
\var_dump($m->firstValue());
```
<!-- HHAPIDOC -->
