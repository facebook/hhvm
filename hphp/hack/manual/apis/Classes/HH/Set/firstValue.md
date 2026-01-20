
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first value in the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function firstValue(): ?Tv;
```




## Returns




+ ` ?Tv ` - The first value in the current [` Set `](/apis/Classes/HH/Set/), or `` null `` if the [` Set `](/apis/Classes/HH/Set/) is
  empty.




## Examples




The following example gets the first value from a [` Set `](/apis/Classes/HH/Set/). An empty [` Set `](/apis/Classes/HH/Set/) will return `` null `` as its first value.




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};
\var_dump($s->firstValue());

$s = Set {};
\var_dump($s->firstValue());
```
<!-- HHAPIDOC -->
