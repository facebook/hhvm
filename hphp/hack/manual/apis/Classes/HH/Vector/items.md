
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) view of the current [` Vector `](/docs/apis/Classes/HH/Vector/)




``` Hack
public function items(): Iterable<Tv>;
```




The [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) returned is one that produces the values from the current
[` Vector `](/docs/apis/Classes/HH/Vector/).




## Returns




+ [` Iterable<Tv> `](/docs/apis/Interfaces/HH/Iterable/) - The [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) view of the current [` Vector `](/docs/apis/Classes/HH/Vector/).




## Examples




This example shows that [` items() `](/docs/apis/Classes/HH/Vector/items/) returns an [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) view of the [` Vector `](/docs/apis/Classes/HH/Vector/). The [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) will produce the values of the [` Vector `](/docs/apis/Classes/HH/Vector/) at the time it's iterated.




``` basic-usage.hack
<<__EntryPoint>>
function basic_usage_main(): void {
  $v = Vector {'red', 'green', 'blue', 'yellow'};

  // Get an Iterable view of the Vector
  $iterable = $v->items();

  // Add another color to the original Vector $v
  $v->add('purple');

  // Print each color using $iterable
  foreach ($iterable as $color) {
    echo $color."\n";
  }
}

// This wouldn't work because the Iterable interface is read-only:
// $iterable->add('orange');
```
<!-- HHAPIDOC -->
