
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first value in the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function firstValue(): ?Tv;
```




## Returns




+ ` ?Tv ` - The first value in the current [` Vector `](/apis/Classes/HH/Vector/), or `` null `` if the
  [` Vector `](/apis/Classes/HH/Vector/) is empty.




## Examples




The following example gets the first value from [` Vector `](/apis/Classes/HH/Vector/). An empty [` Vector `](/apis/Classes/HH/Vector/) will return `` null `` as its first value.




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};
\var_dump($v->firstValue());

$v = Vector {};
\var_dump($v->firstValue());
```
<!-- HHAPIDOC -->
