
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` Map `](/apis/Classes/HH/Map/) starting from a given key location
up to, but not including, the element at the provided length from the
starting key location




``` Hack
public function slice(
  int $start,
  int $len,
): Map<Tk, Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Classes/HH/Map/slice/)`` 2) `` would return the
keys and values at key location 0 and 1.




The returned [` Map `](/apis/Classes/HH/Map/) will always be a proper subset of the current [` Map `](/apis/Classes/HH/Map/).




## Parameters




+ ` int $start ` - The starting key location of the current [` Map `](/apis/Classes/HH/Map/) for the
  returned [` Map `](/apis/Classes/HH/Map/).
+ ` int $len ` - The length of the returned [` Map `](/apis/Classes/HH/Map/).




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - A [` Map `](/apis/Classes/HH/Map/) that is a proper subset of the current [` Map `](/apis/Classes/HH/Map/) starting at
  `` $start `` up to but not including the element ``` $start + $len ```.




## Examples




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
  'purple' => '#663399',
};

// Start at key index 1 ('green') and include 3 elements
$m2 = $m->slice(1, 3);

\var_dump($m2);
```
<!-- HHAPIDOC -->
