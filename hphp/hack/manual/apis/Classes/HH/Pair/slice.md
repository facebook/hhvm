
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` Pair `](/apis/Classes/HH/Pair/) starting from a given key up to,
but not including, the element at the provided length from the starting
key




``` Hack
public function slice(
  int $start,
  int $len,
): ImmVector<mixed>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Classes/HH/Pair/slice/)`` 2) `` would return the
elements at key 0 and 1 (all of the current [` Pair `](/apis/Classes/HH/Pair/) elements).




## Parameters




+ ` int $start ` - The starting key of the current [` Pair `](/apis/Classes/HH/Pair/) to begin the
  returned [` ImmVector `](/apis/Classes/HH/ImmVector/).
+ ` int $len ` - The length of the returned [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Returns




* [` ImmVector<mixed> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) with values from the current [` Pair `](/apis/Classes/HH/Pair/) starting at
  `` $start `` up to but not including the element ``` $start + $len ```.




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

// Get an ImmVector of both values
\var_dump($p->slice(0, 2));

// Get an ImmVector of the first value
\var_dump($p->slice(0, 1));

// Get an ImmVector of the second value
\var_dump($p->slice(1, 1));
```
<!-- HHAPIDOC -->
