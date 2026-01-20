
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an iterator that points to beginning of the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function getIterator(): KeyedIterator<int, Tv>;
```




## Returns




+ [` KeyedIterator<int, `](/apis/Interfaces/HH/KeyedIterator/)`` Tv> `` - A [` KeyedIterator `](/apis/Interfaces/HH/KeyedIterator/) that allows you to traverse the current
  [` Vector `](/apis/Classes/HH/Vector/).




## Examples




This example shows how to get an iterator from a [` Vector `](/apis/Classes/HH/Vector/) and how to consume it:




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Get an iterator for the Vector of colors
$iterator = $v->getIterator();

// Print each color using the iterator
while ($iterator->valid()) {
  echo $iterator->current()."\n";
  $iterator->next();
}
```
<!-- HHAPIDOC -->
