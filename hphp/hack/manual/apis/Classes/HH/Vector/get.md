
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the value at the specified key in the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function get(
  int $key,
): ?Tv;
```




If the key is not present, null is returned. If you would rather have an
exception thrown when a key is not present, use [` at() `](/apis/Classes/HH/Vector/at/) instead.




## Parameters




+ ` int $key `




## Returns




* ` ?Tv ` - The value at the specified key; or `` null `` if the key does not
  exist.




## Examples




This example shows how ` get ` can be used to access an index that may not exist:




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Index 0 is the element 'red'
\var_dump($v->get(0));

// Index 10 doesn't exist
\var_dump($v->get(10));
```
<!-- HHAPIDOC -->
