
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the value at the specified key in the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function at(
  int $key,
): mixed;
```




If the key is not present, an exception is thrown. This essentially means
if you specify a key other than 0 or 1, you will get an exception. If you
don't want an exception to be thrown, use [` get() `](/apis/Classes/HH/Pair/get/) instead.




$v = $p->at($k)" is semantically equivalent to ` $v = $p[$k] `.




## Parameters




+ ` int $key ` - the key from which to retrieve the value.




## Returns




* ` mixed ` - The value at the specified key; or an exception if the key does
  not exist.




## Examples




This example prints the first and second values of the [` Pair `](/apis/Classes/HH/Pair/):




``` existing-key.hack
$p = Pair {'foo', -1.5};

// Print the first element
\var_dump($p->at(0));

// Print the second element
\var_dump($p->at(1));
```




This example throws an ` OutOfBoundsException ` because a [` Pair `](/apis/Classes/HH/Pair/) only has the indexes `` 0 `` and ``` 1 ```:




``` missing-key.hack
$p = Pair {'foo', -1.5};

// Index 2 doesn't exist because pairs always have exactly two elements
\var_dump($p->at(2));
```
<!-- HHAPIDOC -->
