
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the first `` $n `` values of the current
[` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function take(
  int $n,
): Vector<Tv>;
```




The returned [` Vector `](/apis/Classes/HH/Vector/) will always be a subset (but not necessarily a
proper subset) of the current [` Vector `](/apis/Classes/HH/Vector/). If `` $n `` is greater than the length
of the current [` Vector `](/apis/Classes/HH/Vector/), the returned [` Vector `](/apis/Classes/HH/Vector/) will contain all elements of
the current [` Vector `](/apis/Classes/HH/Vector/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the returned
  [` Vector `](/apis/Classes/HH/Vector/).




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) that is a subset of the current [` Vector `](/apis/Classes/HH/Vector/) up to `` $n ``
  elements.




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Take the first two elements
$take2 = $v->take(2);

\var_dump($take2);
```
<!-- HHAPIDOC -->
