
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Map `](/apis/Classes/HH/Map/) containing the values after the `` n ``-th element of the
current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function skip(
  int $n,
): Map<Tk, Tv>;
```




The returned [` Map `](/apis/Classes/HH/Map/) will always be a proper subset of the current [` Map `](/apis/Classes/HH/Map/).




` n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element to be skipped; the `` $n+1 `` element will be the
  first one in the returned [` Map `](/apis/Classes/HH/Map/).




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - A [` Map `](/apis/Classes/HH/Map/) that is a proper subset of the current [` Map `](/apis/Classes/HH/Map/) containing
  values after the specified `` n ``-th element.




## Examples




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Create a new Map after skipping the first two elements ('red' and 'green')
$skip2 = $m->skip(2);

\var_dump($skip2);
```
<!-- HHAPIDOC -->
