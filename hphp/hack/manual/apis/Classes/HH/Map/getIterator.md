
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an iterator that points to beginning of the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function getIterator(): KeyedIterator<Tk, Tv>;
```




## Returns




+ [` KeyedIterator<Tk, `](/apis/Interfaces/HH/KeyedIterator/)`` Tv> `` - A [` KeyedIterator `](/apis/Interfaces/HH/KeyedIterator/) that allows you to traverse the current [` Map `](/apis/Classes/HH/Map/).




## Examples




This example shows how to get a [` KeyedIterator `](/apis/Interfaces/HH/KeyedIterator/) from a [` Map `](/apis/Classes/HH/Map/) and how to consume it:




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Get an iterator for the Map of colors to hex codes
$iterator = $m->getIterator();

// Print each color (key) and hex code (value) using the iterator
while ($iterator->valid()) {
  echo $iterator->key().' => '.$iterator->current()."\n";
  $iterator->next();
}
```
<!-- HHAPIDOC -->
