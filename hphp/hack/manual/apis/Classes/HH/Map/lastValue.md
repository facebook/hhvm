
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last value in the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function lastValue(): ?Tv;
```




## Returns




+ ` ?Tv ` - The last value in the current [` Map `](/apis/Classes/HH/Map/), or `` null `` if the [` Map `](/apis/Classes/HH/Map/) is
  empty.




## Examples




This example shows how [` lastValue() `](/apis/Classes/HH/Map/lastValue/) can be used even when a [` Map `](/apis/Classes/HH/Map/) may be empty:




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};
\var_dump($m->lastValue());

$m = Map {};
\var_dump($m->lastValue());
```
<!-- HHAPIDOC -->
