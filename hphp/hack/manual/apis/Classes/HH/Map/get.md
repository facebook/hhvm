
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the value at the specified key in the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function get(
  Tk $key,
): ?Tv;
```




If the key is not present, ` null ` is returned. If you would rather have an
exception thrown when a key is not present, then use [` at() `](/apis/Classes/HH/Map/at/).




## Parameters




+ ` Tk $key `




## Returns




* ` ?Tv ` - The value at the specified key; or `` null `` if the key does not
  exist.




## Examples




This example shows how ` get ` can be used to access the value at a key that may not exist:




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Prints the value at key 'red'
\var_dump($m->get('red'));

// Prints NULL since key 'blurple' doesn't exist
\var_dump($m->get('blurple'));
```
<!-- HHAPIDOC -->
