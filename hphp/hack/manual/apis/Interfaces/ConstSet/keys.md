
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) containing the values of the current [` ConstSet `](/apis/Interfaces/ConstSet/)




``` Hack
public function keys(): ConstVector<arraykey>;
```




Sets don't have keys, so this will return the values.




This method is interchangeable with [` values() `](/apis/Interfaces/ConstSet/values/).




## Returns




+ [` ConstVector<arraykey> `](/apis/Interfaces/ConstVector/) - a [` ConstVector `](/apis/Interfaces/ConstVector/) (integer-indexed) containing the values of the
  current [` ConstSet `](/apis/Interfaces/ConstSet/).
<!-- HHAPIDOC -->
