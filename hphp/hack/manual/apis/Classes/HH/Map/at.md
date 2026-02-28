
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the value at the specified key in the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function at(
  Tk $key,
): Tv;
```




If the key is not present, an exception is thrown. If you don't want an
exception to be thrown, use [` get() `](/apis/Classes/HH/Map/get/) instead.




` $v = $map->at($k) ` is equivalent to `` $v = $map[$k] ``.




## Parameters




+ ` Tk $key `




## Returns




* ` Tv ` - The value at the specified key; or an exception if the key does
  not exist.




## Examples




This example prints the values at the keys ` red ` and `` green `` in the [` Map `](/apis/Classes/HH/Map/):




``` existing-key.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Print the value at the key 'red'
\var_dump($m->at('red'));

// Print the value at the key 'yellow'
\var_dump($m->at('yellow'));
```




This example throws an ` OutOfBoundsException ` because the [` Map `](/apis/Classes/HH/Map/) has no key 'blurple':




``` missing-key.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Key 'blurple' doesn't exist (this will throw an exception)
\var_dump($m->at('blurple'));
```
<!-- HHAPIDOC -->
