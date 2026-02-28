
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Set `](/apis/Classes/HH/Set/) containing the first `` n `` values of the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function take(
  int $n,
): Set<Tv>;
```




The returned [` Set `](/apis/Classes/HH/Set/) will always be a proper subset of the current [` Set `](/apis/Classes/HH/Set/).




` n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the [` Set `](/apis/Classes/HH/Set/).




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - A [` Set `](/apis/Classes/HH/Set/) that is a proper subset of the current [` Set `](/apis/Classes/HH/Set/) up to `` n ``
  elements.




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

// Take the first two elements
$take2 = $s->take(2);

\var_dump($take2);
```
<!-- HHAPIDOC -->
