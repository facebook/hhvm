
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) view of the current [` Map `](/docs/apis/Classes/HH/Map/)




``` Hack
public function items(): Iterable<Pair<Tk, Tv>>;
```




The [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) returned is one that produces the key/values from the
current [` Map `](/docs/apis/Classes/HH/Map/).




## Returns




+ [` Iterable<Pair<Tk, `](/docs/apis/Interfaces/HH/Iterable/)`` Tv>> `` - The [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) view of the current [` Map `](/docs/apis/Classes/HH/Map/).




## Examples




This example shows that [` items() `](/docs/apis/Classes/HH/Map/items/) returns an [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) view of the [` Map `](/docs/apis/Classes/HH/Map/). The [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) will produce the key-value pairs of the [` Map `](/docs/apis/Classes/HH/Map/) at the time it's iterated.




``` basic-usage.hack
<<__EntryPoint>>
function basic_usage_main(): void {
  $m = Map {
    'red' => '#ff0000',
    'green' => '#00ff00',
    'blue' => '#0000ff',
    'yellow' => '#ffff00',
  };

  // Get an Iterable view of the Map
  $items = $m->items();

  // Add another color to the original Map $m
  $m->add(Pair {'purple', '#663399'});

  // Print each color and hex code using the Iterable
  foreach ($items as $key_value_pair) {
    echo $key_value_pair[0].' => '.$key_value_pair[1]."\n";
  }
}

// This wouldn't work because the Iterable interface is read-only:
// $iterable->add(Pair {'purple', '#ff6600'});
```
<!-- HHAPIDOC -->
