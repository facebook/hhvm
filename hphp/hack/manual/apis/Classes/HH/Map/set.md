
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Stores a value into the current [` Map `](/apis/Classes/HH/Map/) with the specified key, overwriting
the previous value associated with the key




``` Hack
public function set(
  Tk $key,
  Tv $value,
): Map<Tk, Tv>;
```




This method is equivalent to [` Map::add() `](/apis/Classes/HH/Map/add/). If the key to set does not exist,
it is created. This is inconsistent with, for example, [` Vector::set() `](/apis/Classes/HH/Vector/set/)
where if the key is not found, an exception is thrown.




` $map->set($k,$v) ` is equivalent to `` $map[$k] = $v `` (except that [` set() `](/apis/Classes/HH/Map/set/)
returns the current [` Map `](/apis/Classes/HH/Map/)).




Future changes made to the current [` Map `](/apis/Classes/HH/Map/) ARE reflected in the returned
[` Map `](/apis/Classes/HH/Map/), and vice-versa.




## Parameters




+ ` Tk $key `
+ ` Tv $value `




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - Returns itself.




## Examples




Since [` Map::set() `](/apis/Classes/HH/Map/set/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $m ` itself, you can chain a bunch of [` set() `](/apis/Classes/HH/Map/set/) calls together.




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Set the value at key 'red'
$m->set('red', 'rgb(255, 0, 0)');

// Set the values at keys 'green' and 'blue'
$m->set('green', 'rgb(0, 255, 0)')
  ->set('blue', 'rgb(0, 0, 255)');

\var_dump($m);
```
<!-- HHAPIDOC -->
