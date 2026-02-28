
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) view of the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function items(): Iterable<mixed>;
```




The [` Iterable `](/apis/Interfaces/HH/Iterable/) returned is one that produces the values from the current
[` Pair `](/apis/Classes/HH/Pair/).




## Returns




+ [` Iterable<mixed> `](/apis/Interfaces/HH/Iterable/) - The [` Iterable `](/apis/Interfaces/HH/Iterable/) view of the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

// Get an Iterable view of the Pair
$iterable = $p->items();

// Print both values in the Iterable
foreach ($iterable as $value) {
  echo (string)$value."\n";
}
```
<!-- HHAPIDOC -->
