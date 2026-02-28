
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the first `` n `` values of the current
[` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function take(
  int $n,
): ImmVector<mixed>;
```




` n ` is 1-based. So the first element is 1, the second 2. There is no
element 3 in a [` Pair `](/apis/Classes/HH/Pair/), but if you specify an element greater than 2, it
will just return all elements in the [` Pair `](/apis/Classes/HH/Pair/).




## Parameters




+ ` int $n ` - The last element that will be included in the current [` Pair `](/apis/Classes/HH/Pair/).




## Returns




* [` ImmVector<mixed> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the first `` n `` values of the current
  [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

// Taking 0 returns an empty ImmVector
\var_dump($p->take(0));

// Taking 1 returns an ImmVector of the first value
\var_dump($p->take(1));

// Taking 2 (or more) returns an ImmVector containing both values
\var_dump($p->take(2));
```
<!-- HHAPIDOC -->
