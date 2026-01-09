
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes the specified key (and associated value) from the current [` Map `](/docs/apis/Classes/HH/Map/)




``` Hack
public function removeKey(
  Tk $key,
): Map<Tk, Tv>;
```




This method is interchangeable with [` remove() `](/docs/apis/Classes/HH/Map/remove/).




Future changes made to the current [` Map `](/docs/apis/Classes/HH/Map/) ARE reflected in the returned
[` Map `](/docs/apis/Classes/HH/Map/), and vice-versa.




## Parameters




+ ` Tk $key `




## Returns




* [` Map<Tk, `](/docs/apis/Classes/HH/Map/)`` Tv> `` - Returns itself.




## Examples




Since [` Map::removeKey() `](/docs/apis/Classes/HH/Map/removeKey/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $m ` itself, you can chain a bunch of [` removeKey() `](/docs/apis/Classes/HH/Map/removeKey/) calls together.




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Remove key 'red'
$m->removeKey('red');
\var_dump($m);

// Remove keys 'green' and 'blue'
$m->removeKey('green')->removeKey('blue');
\var_dump($m);
```
<!-- HHAPIDOC -->
