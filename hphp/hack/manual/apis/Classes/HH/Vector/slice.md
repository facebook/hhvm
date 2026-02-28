
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` Vector `](/apis/Classes/HH/Vector/) starting from a given key up to,
but not including, the element at the provided length from the starting key




``` Hack
public function slice(
  int $start,
  int $len,
): Vector<Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Classes/HH/Vector/slice/)`` 2) `` would return the
elements at keys 0 and 1.




The returned [` Vector `](/apis/Classes/HH/Vector/) will always be a subset (but not necessarily a
proper subset) of the current [` Vector `](/apis/Classes/HH/Vector/). If `` $start `` is greater than or
equal to the length of the current [` Vector `](/apis/Classes/HH/Vector/), the returned [` Vector `](/apis/Classes/HH/Vector/) will
contain no elements.  If `` $start `` + ``` $len ``` is greater than or equal to the
length of the current [` Vector `](/apis/Classes/HH/Vector/), the returned [` Vector `](/apis/Classes/HH/Vector/) will contain the
elements from `` $start `` to the end of the current [` Vector `](/apis/Classes/HH/Vector/).




If either ` $start ` or `` $len `` is negative, an exception is thrown.




## Parameters




+ ` int $start ` - The starting key of the current [` Vector `](/apis/Classes/HH/Vector/) at which to begin
  the returned [` Vector `](/apis/Classes/HH/Vector/).
+ ` int $len ` - The length of the returned [` Vector `](/apis/Classes/HH/Vector/).




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) that is a subset of the current [` Vector `](/apis/Classes/HH/Vector/) starting
  at `` $start `` up to but not including the element ``` $start + $len ```.




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Start at index 1 ('green') and include 3 elements
$v2 = $v->slice(1, 3);

\var_dump($v2);
```
<!-- HHAPIDOC -->
