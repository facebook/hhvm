
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Map `](/apis/Classes/HH/Map/) containing the first `` n `` key/values of the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function take(
  int $n,
): Map<Tk, Tv>;
```




The returned [` Map `](/apis/Classes/HH/Map/) will always be a proper subset of the current [` Map `](/apis/Classes/HH/Map/).




` n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the [` Map `](/apis/Classes/HH/Map/).




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - A [` Map `](/apis/Classes/HH/Map/) that is a proper subset of this [` Map `](/apis/Classes/HH/Map/) up to `` n `` elements.




## Examples




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
  'purple' => '#663399',
};

// Take the first two elements
$take2 = $m->take(2);

\var_dump($take2);
```
<!-- HHAPIDOC -->
