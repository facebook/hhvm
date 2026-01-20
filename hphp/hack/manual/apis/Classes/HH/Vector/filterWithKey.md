
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current [` Vector `](/apis/Classes/HH/Vector/) that meet
a supplied condition applied to its keys and values




``` Hack
public function filterWithKey(
  (function(int, Tv): bool) $callback,
): Vector<Tv>;
```




[` filterWithKey() `](/apis/Classes/HH/Vector/filterWithKey/)'s result contains only values whose key/value pairs
satisfy the provided criterion; unlike [` mapWithKey() `](/apis/Classes/HH/Vector/mapWithKey/), which contains
results derived from every key/value pair in the original [` Vector `](/apis/Classes/HH/Vector/).




## Parameters




+ ` (function(int, Tv): bool) $callback `




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current [` Vector `](/apis/Classes/HH/Vector/) for
  which a user-specified test condition returns true when applied
  to the corresponding key/value pairs.




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow', 'purple'};

// Only include elements with an odd index
$odd_elements = $v->filterWithKey(($index, $color) ==> ($index % 2) !== 0);

\var_dump($odd_elements);
```
<!-- HHAPIDOC -->
