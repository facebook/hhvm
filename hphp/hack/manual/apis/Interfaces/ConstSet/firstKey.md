
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first "key" in the current [` ConstSet `](/apis/Interfaces/ConstSet/)




``` Hack
public function firstKey(): ?arraykey;
```




Since sets do not have keys, it returns the first value.




This method is interchangeable with [` firstValue() `](/apis/Interfaces/ConstSet/firstValue/).




## Returns




+ ` ?arraykey ` - The first value in the current [` ConstSet `](/apis/Interfaces/ConstSet/), or `` null `` if the
  [` ConstSet `](/apis/Interfaces/ConstSet/) is empty.
<!-- HHAPIDOC -->
