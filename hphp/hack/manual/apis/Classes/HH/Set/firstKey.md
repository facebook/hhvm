
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first "key" in the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function firstKey(): ?arraykey;
```




Since [` Set `](/apis/Classes/HH/Set/)s do not have keys, it returns the first value.




This method is interchangeable with [` firstValue() `](/apis/Classes/HH/Set/firstValue/).




## Returns




+ ` ?arraykey ` - The first value in the current [` Set `](/apis/Classes/HH/Set/), or `` null `` if the [` Set `](/apis/Classes/HH/Set/) is
  empty.




## Examples




This example shows that ` firstKey ` returns the first value in the [` Set `](/apis/Classes/HH/Set/). An empty [` Set `](/apis/Classes/HH/Set/) will return `` null `` as its first key.




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};
\var_dump($s->firstKey());

$s = Set {};
\var_dump($s->firstKey());
```
<!-- HHAPIDOC -->
