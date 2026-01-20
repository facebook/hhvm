
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an iterator that points to beginning of the current [` ImmSet `](/apis/Classes/HH/ImmSet/)




``` Hack
public function getIterator(): KeyedIterator<arraykey, Tv>;
```




The keys and values when iterating through the [` KeyedIterator `](/apis/Interfaces/HH/KeyedIterator/) will be
identical since [` ImmSet `](/apis/Classes/HH/ImmSet/)s have no keys, the values are used as keys.




## Returns




+ [` KeyedIterator<arraykey, `](/apis/Interfaces/HH/KeyedIterator/)`` Tv> `` - A [` KeyedIterator `](/apis/Interfaces/HH/KeyedIterator/) that allows you to traverse the current
  [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Examples




See [` Set::getIterator `](/apis/Classes/HH/Set/getIterator/#examples) for usage examples.
<!-- HHAPIDOC -->
