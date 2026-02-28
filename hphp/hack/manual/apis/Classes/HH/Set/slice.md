
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` Set `](/apis/Classes/HH/Set/) starting from a given key up to, but
not including, the element at the provided length from the starting key




``` Hack
public function slice(
  int $start,
  int $len,
): Set<Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Classes/HH/Set/slice/)`` 2) `` would return the
elements at key 0 and 1.




The returned [` Set `](/apis/Classes/HH/Set/) will always be a proper subset of the current [` Set `](/apis/Classes/HH/Set/).




## Parameters




+ ` int $start ` - The starting value in the current [` Set `](/apis/Classes/HH/Set/) for the returned
  [` Set `](/apis/Classes/HH/Set/).
+ ` int $len ` - The length of the returned [` Set `](/apis/Classes/HH/Set/).




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - A [` Set `](/apis/Classes/HH/Set/) that is a proper subset of the current [` Set `](/apis/Classes/HH/Set/) starting at
  `` $start `` up to but not including the element ``` $start + $len ```.




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

// Start at index 1 ('green') and include 3 elements
$s2 = $s->slice(1, 3);

\var_dump($s2);
```
<!-- HHAPIDOC -->
