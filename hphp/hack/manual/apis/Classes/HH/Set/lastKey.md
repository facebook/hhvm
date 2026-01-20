
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last "key" in the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function lastKey(): ?arraykey;
```




Since [` Set `](/apis/Classes/HH/Set/)s do not have keys, it returns the last value.




This method is interchangeable with [` lastValue() `](/apis/Classes/HH/Set/lastValue/).




## Returns




+ ` ?arraykey ` - The last value in the current [` Set `](/apis/Classes/HH/Set/), or `` null `` if the current
  [` Set `](/apis/Classes/HH/Set/) is empty.




## Examples




This example shows that ` lastKey ` returns the last value in the [` Set `](/apis/Classes/HH/Set/). An empty [` Set `](/apis/Classes/HH/Set/) will return `` null `` as its last key/value.




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};
\var_dump($s->lastKey());

$s = Set {};
\var_dump($s->lastKey());
```
<!-- HHAPIDOC -->
