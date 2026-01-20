
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last value in the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function lastValue(): ?Tv;
```




## Returns




+ ` ?Tv ` - The last value in the current [` Set `](/apis/Classes/HH/Set/), or `` null `` if the current
  [` Set `](/apis/Classes/HH/Set/) is empty.




## Examples




This example shows how [` lastValue() `](/apis/Classes/HH/Set/lastValue/) can be used even when a [` Set `](/apis/Classes/HH/Set/) may be empty:




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};
\var_dump($s->lastValue());

$s = Set {};
\var_dump($s->lastValue());
```
<!-- HHAPIDOC -->
