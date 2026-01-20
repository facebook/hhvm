
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the value at the specified key in the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function at(
  int $key,
): Tv;
```




If the key is not present, an exception is thrown. If you don't want an
exception to be thrown, use [` get() `](/apis/Classes/HH/Vector/get/) instead.




` $v = $vec->at($k) ` is semantically equivalent to `` $v = $vec[$k] ``.




## Parameters




+ ` int $key `




## Returns




* ` Tv ` - The value at the specified key; or an exception if the key does
  not exist.




## Examples




This example prints the first and last values of the [` Vector `](/apis/Classes/HH/Vector/):




``` existing-key.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Print the first element
\var_dump($v->at(0));

// Print the last element
\var_dump($v->at(3));
```




This example throws an ` OutOfBoundsException ` because the [` Vector `](/apis/Classes/HH/Vector/) has no index 10:




``` missing-key.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Index 10 doesn't exist (this will throw an exception)
\var_dump($v->at(10));
```
<!-- HHAPIDOC -->
