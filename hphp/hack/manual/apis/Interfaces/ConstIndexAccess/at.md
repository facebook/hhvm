
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the value at the specified key in the current collection




``` Hack
public function at(
  Tk $k,
): Tv;
```




If the key is not present, an exception is thrown. If you don't want an
exception to be thrown, use [` get() `](/apis/Interfaces/ConstIndexAccess/get/) instead.




` $v = $vec->at($k) ` is semantically equivalent to `` $v = $vec[$k] ``.




## Parameters




+ ` Tk $k ` - the key from which to retrieve the value.




## Returns




* ` Tv ` - The value at the specified key; or an exception if the key does
  not exist.
<!-- HHAPIDOC -->
