
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values of the current
[` MutableSet `](/apis/Interfaces/MutableSet/)




``` Hack
public function keys(): MutableVector<arraykey>;
```




Sets don't have keys, so this will return the values.




This method is interchangeable with [` values() `](/apis/Interfaces/MutableSet/values/).




## Returns




+ [` MutableVector<arraykey> `](/apis/Interfaces/MutableVector/) - a [` MutableVector `](/apis/Interfaces/MutableVector/) (integer-indexed) containing the values of the
  current [` MutableSet `](/apis/Interfaces/MutableSet/).
<!-- HHAPIDOC -->
