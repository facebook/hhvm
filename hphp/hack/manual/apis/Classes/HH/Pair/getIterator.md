
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an iterator that points to beginning of the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function getIterator(): KeyedIterator<int, mixed>;
```




## Returns




+ [` KeyedIterator<int, `](/apis/Interfaces/HH/KeyedIterator/)`` mixed> `` - A [` KeyedIterator `](/apis/Interfaces/HH/KeyedIterator/) that allows you to traverse the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




This example shows how to get an iterator from a [` Pair `](/apis/Classes/HH/Pair/) and how to consume it:




``` basic-usage.hack
$p = Pair {'foo', -1.5};

// Get a KeyedIterator for the Pair
$iterator = $p->getIterator();

// Print both keys and values
while ($iterator->valid()) {
  echo $iterator->key().' => '.(string)$iterator->current()."\n";
  $iterator->next();
}
```
<!-- HHAPIDOC -->
