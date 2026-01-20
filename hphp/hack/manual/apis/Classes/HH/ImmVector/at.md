
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the value at the specified key in the current [` ImmVector `](/apis/Classes/HH/ImmVector/)




``` Hack
public function at(
  int $key,
): Tv;
```




If the key is not present, an exception is thrown. If you don't want an
exception to be thrown, use [` get() `](/apis/Classes/HH/ImmVector/get/) instead.




` $v = $vec->at($k) ` is semantically equivalent to `` $v = $vec[$k] ``.




## Parameters




+ ` int $key `




## Returns




* ` Tv ` - The value at the specified key; or an exception if the key does
  not exist.




## Examples




See [` Vector::at `](/apis/Classes/HH/Vector/at/#examples) for usage examples.
<!-- HHAPIDOC -->
