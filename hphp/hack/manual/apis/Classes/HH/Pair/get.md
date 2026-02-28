
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the value at the specified key in the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function get(
  int $key,
): mixed;
```




If the key is not present, ` null ` is returned. If you would rather have an
exception thrown when a key is not present, then use [` at() `](/apis/Classes/HH/Pair/at/).




## Parameters




+ ` int $key ` - the key from which to retrieve the value.




## Returns




* ` mixed ` - The value at the specified key; or `` null `` if the key does not
  exist.




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

// Print the first value
\var_dump($p->get(0));

// Print the second value
\var_dump($p->get(1));

// Print NULL since index 2 doesn't exist in a Pair
\var_dump($p->get(2));
```
<!-- HHAPIDOC -->
