
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Set `](/apis/Classes/HH/Set/) containing the values after the `` n ``-th element of the
current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function skip(
  int $n,
): Set<Tv>;
```




The returned [` Set `](/apis/Classes/HH/Set/) will always be a proper subset of the current [` Set `](/apis/Classes/HH/Set/).




` n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element to be skipped; the `` $n+1 `` element will be
  the first one in the returned [` Set `](/apis/Classes/HH/Set/).




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - A [` Set `](/apis/Classes/HH/Set/) that is a proper subset of the current [` Set `](/apis/Classes/HH/Set/) containing
  values after the specified `` n ``-th element.




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

// Create a new Set after skipping the first two elements ('red' and 'green')
$skip2 = $s->skip(2);

\var_dump($skip2);
```
<!-- HHAPIDOC -->
