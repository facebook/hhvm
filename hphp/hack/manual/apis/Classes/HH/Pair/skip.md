
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values after the `` n ``-th element of
the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function skip(
  int $n,
): ImmVector<mixed>;
```




` n ` is 1-based. So the first element is 1, the second 2, etc. There is no
element 3 in a [` Pair `](/apis/Classes/HH/Pair/), but if you specify an element greater than or equal
to 2, it will just return empty. If you specify 0, it will return all the
elements in the [` Pair `](/apis/Classes/HH/Pair/).




## Parameters




+ ` int $n ` - The last element to be skipped; the `` $n+1 `` element will be the
  first one in the returned [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Returns




* [` ImmVector<mixed> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) that contains values after the specified `` n ``-th
  element in the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

// Skipping 0 returns an ImmVector of both values
\var_dump($p->skip(0));

// Skipping 1 returns an ImmVector of the second value
\var_dump($p->skip(1));

// Skipping more than 1 returns an empty ImmVector
\var_dump($p->skip(2));
```
<!-- HHAPIDOC -->
